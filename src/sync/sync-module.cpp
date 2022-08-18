#include "sync/sync-module.hpp"
// #include "sync/recursive-fetcher.hpp"

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

SyncModule::SyncModule(const SyncOptions &options, const SecurityOptions& secOps, ndn::Face& face,
                       const ExistLocalRecord& exist, const YieldRecordCallback& yield)
  : m_syncOptions(options)
  , m_secOptions(secOps)
  , m_face(face)
  , m_existCb(exist)
  , m_yieldCb(yield)
{
  // TODO: move to ecdsa later
  m_svs = std::make_shared<LedgerSVS>(m_syncOptions.prefix, 
                                      Name(m_syncOptions.prefix).append(m_syncOptions.id),
                                      m_face, std::bind(&SyncModule::onMissingData, this, _1), m_secOptions);
}

std::tuple<NodeID, SeqNo>
SyncModule::parseDataName(const Name& name)
{
  return std::make_tuple<NodeID, SeqNo>(name.getPrefix(-1)
                                            .getSubName(m_syncOptions.prefix.size()),
                                        name.get(-1).toNumber());
}

void
SyncModule::onMissingData(const std::vector<MissingDataInfo>& vectors)
{ 
  // an accumlator to collect all missing records along the way
  auto acc = std::make_shared<std::list<Record>>();
  for (auto& v : vectors) {
    recursiveFetcher(v.nodeId, v.high, acc); 
  }
}

void
SyncModule::recursiveFetcher(const NodeID& nid, const SeqNo& s, std::shared_ptr<std::list<Record>> acc)
{
  auto search = [this, acc] (const Name& n) {
    for (auto& r : *acc) {
      if (r.getName() == n) return true;
    }
    return m_existCb(n);
  };

  NDN_LOG_TRACE("trying getting data " << m_svs->getDataName(nid, s));
  m_svs->fetchData(nid, s, [this, acc, search] (const ndn::Data& data) {
    NDN_LOG_DEBUG("getting data " << data.getName());

    Record record(data.getName(), data.getContent());
    if (!search(record.getName())) acc->push_front(record);

    if (record.isGenesis()) {
      NDN_LOG_INFO(record.getName() << " is an new genesis record");
    }

    // check the pointer first
    bool allExist = true;
    for (auto& p : record.getPointers()) {
      if (search(p)) {
        // then we don't need to care
        NDN_LOG_TRACE(p << " already exists");
      }
      else {
        allExist = false;
        NDN_LOG_DEBUG("discover " << p);
        // we need to fetch it
        auto tuple = parseDataName(p);
        recursiveFetcher(std::get<0>(tuple), std::get<1>(tuple), acc);
      }
    }

    if (allExist) {
      // clean the acc
      for (auto& r : *acc) m_yieldCb(r);
      acc->clear();
    }
  });
}

void
SyncModule::publishRecord(Record& record)
{ 
  m_svs->publishData(*record.prepareContent(), ndn::time::milliseconds(3000));
}

} // namespace cledger::sync