#include "sync/sync-module.hpp"

namespace cledger::sync {
NDN_LOG_INIT(cledger.sync);

SyncModule::SyncModule(const SyncOptions &options, const SecurityOptions& secOps, ndn::Face& face,
                       storage::Interface storageIntf, const YieldRecordCallback& yield)
  : m_syncOptions(options)
  , m_secOptions(secOps)
  , m_face(face)
  , m_storageIntf(storageIntf)
  , m_yieldCb(yield)
{
  m_svs = std::make_shared<LedgerSVSBase>(m_syncOptions.prefix, 
                                          m_syncOptions.id,
                                          m_face, std::bind(&SyncModule::onMissingData, this, _1), m_secOptions,
                                          std::make_shared<LedgerSVSDataStore>(m_storageIntf));
}

std::tuple<NodeID, SeqNo>
SyncModule::parseDataName(const Name& name)
{
  return std::make_tuple<NodeID, SeqNo>(name.getPrefix(-1 - m_syncOptions.prefix.size()),
                                        name.get(-1).toNumber());
}

void
SyncModule::onMissingData(const std::vector<MissingDataInfo>& vectors)
{ 
  // an accumlator to collect all missing records along the way
  for (auto& v : vectors) {
    for (SeqNo curr = v.low; curr <= v.high; curr++) {
      recursiveFetcher(m_svs->getDataName(v.nodeId, curr));
    }
  }
}

void
SyncModule::fetcher(const NodeID& nid, const SeqNo& s)
{
  auto searchStorage = [this] (const Name& n) {
    try {
      m_storageIntf.getter(n);
      return true;
    }
    catch (std::exception& e) {
      return false;
    }
  };

  // check again if exist in acc or storage
  if (searchStorage(m_svs->getDataName(nid, s))) {
    NDN_LOG_TRACE("Already fetched " << m_svs->getDataName(nid, s));
    return;
  }

  NDN_LOG_TRACE("Trying getting data " << m_svs->getDataName(nid, s));
  m_svs->fetchData(nid, s, [this, searchStorage] (const ndn::Data& data) {
    NDN_LOG_DEBUG("Getting data " << data.getName());

    m_storageIntf.adder(data.getName(), data.wireEncode());
    NDN_LOG_TRACE("Yielding " << data.getName());
    m_yieldCb(Record(data.getName(), data.getContent()));

  },
  MAX_RETRIES);
}
void
SyncModule::recursiveFetcher(const Name& recordName)
{
  auto searchStorage = [this] (const Name& n) {
    try {
      m_storageIntf.getter(n);
      return true;
    }
    catch (std::exception& e) {
      return false;
    }
  };

  // check again if exist in acc or storage
  if (searchStorage(recordName)) {
    NDN_LOG_TRACE("Already fetched " << recordName);
    return;
  }

  NDN_LOG_TRACE("Trying getting data " << recordName);
  auto tuple = parseDataName(recordName);
  m_svs->fetchData(std::get<0>(tuple), std::get<1>(tuple), [this, searchStorage] (const ndn::Data& data) {
    NDN_LOG_DEBUG("Getting data " << data.getName());

    if (!searchStorage(data.getName())) {
      m_storageIntf.adder(data.getName(), data.wireEncode());
    }

    NDN_LOG_TRACE("Yielding " << data.getName());
    Record record = Record(data.getName(), data.getContent());
    m_yieldCb(record);
  
    if (record.isGenesis()) {
      NDN_LOG_INFO(data.getName() << " is an new genesis record");
    }
    else {
      // keep fetching
      for (auto& pointer : record.getPointers()) {
        recursiveFetcher(pointer);
      }
    }
  },
  MAX_RETRIES);
}

Name
SyncModule::publishRecord(Record& record)
{ 
  SeqNo seq = m_svs->publishData(*record.prepareContent(), ndn::time::milliseconds(3000));
  auto puiblishedName = m_svs->getDataName(m_syncOptions.id, seq);
  NDN_LOG_DEBUG("Published " << puiblishedName);
  return puiblishedName;
}

} // namespace cledger::sync