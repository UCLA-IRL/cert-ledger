#include "sync/sync-module.hpp"

namespace cledger::sync {
NDN_LOG_INIT(cledger.sync);

LedgerSVS::LedgerSVS(const Name& syncPrefix,
                      const Name& nodePrefix,
                      ndn::Face& face,
                      const UpdateCallback& updateCallback,
                      const SecurityOptions& securityOptions,
                      std::shared_ptr<DataStore> dataStore)
  : SVSyncBase(syncPrefix, Name(nodePrefix).append(syncPrefix), nodePrefix,
                face, updateCallback, securityOptions, std::move(dataStore))
{}

Name
LedgerSVS::getDataName(const NodeID& nid, const SeqNo& seqNo)
{
  return Name(m_syncPrefix).append(nid).appendNumber(seqNo);
}

SyncModule::SyncModule(const SyncOptions &options, ndn::Face& face,
                       const ExistLocalRecord& exist, const YieldRecordCallback& yield)
  : m_syncOptions(options)
  , m_face(face)
  , m_secOptions(m_keyChain)
  , m_existCb(exist)
  , m_yieldCb(yield)
{
  // TODO: move to ecdsa later
  m_signingInfo.setSha256Signing();
  m_secOptions.interestSigner->signingInfo.setSigningHmacKey("dGhpcyBpcyBhIHNlY3JldCBtZXNzYWdl");
  m_svs = std::make_shared<LedgerSVS>(m_syncOptions.prefix, 
                                      Name(m_syncOptions.prefix).append(m_syncOptions.id),
                                      m_face, std::bind(&SyncModule::onMissingData, this, _1), m_secOptions);
}

std::tuple<NodeID, SeqNo>
SyncModule::parseDataName(const Name& name)
{
  return std::make_tuple<NodeID, SeqNo>(name.getPrefix(-2), name.get(-1).toNumber());
}

void
SyncModule::onMissingData(const std::vector<MissingDataInfo>& vectors)
{ 
  // an accumlator to collect all missing records along the way
  std::list<Record> acc;
  for (auto& v : vectors) {
    for (SeqNo s = v.low; s <= v.high; ++s) {
      recursiveFetcher(v.nodeId, s, acc);
    }      
  }

  NDN_LOG_DEBUG("collected " << acc.size() << " missing records, yeild...");
  m_yieldCb(acc);
}

void
SyncModule::recursiveFetcher(const NodeID& nid, const SeqNo& s, std::list<Record>& acc)
{
  auto accSearch = [&acc] (const Name& n) {
    for (auto& i : acc) {
      if (i.getName() == n) {
        return false;
      }
    }
    return true;
  };

  m_svs->fetchData(nid, s, [this, &acc, accSearch] (const ndn::Data& data) {
    Record record(data.getName(), data.getContent());

    // check the pointer first
    for (auto& p : record.getPointers()) {
      if (m_existCb(p) || accSearch(data.getName())) {
        // then we don't need to care
      }
      else {
        // we need to fetch it
        auto tuple = parseDataName(p);
        recursiveFetcher(std::get<0>(tuple), std::get<1>(tuple), acc);
      }
    }

    // check the record itself
    if (m_existCb(data.getName()) || accSearch(data.getName())) {
      // then we don't need to do anything
    }
    else {
      acc.push_back(record);
    }
  });  
}

void
SyncModule::publishRecord(Record& record)
{ 
  m_svs->publishData(*record.prepareContent(), ndn::time::milliseconds(3000));
}

} // namespace cledger::sync