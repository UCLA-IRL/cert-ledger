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
  auto acc = std::make_shared<std::list<Data>>();
  for (auto& v : vectors) {
    recursiveFetcher(v.nodeId, v.high, acc); 
  }
}

void
SyncModule::recursiveFetcher(const NodeID& nid, const SeqNo& s, std::shared_ptr<std::list<Data>> acc)
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

  auto searchAccOrStorage = [searchStorage, acc] (const Name& n) {
    for (auto& r : *acc) {
      if (r.getName() == n) return true;
    }
    return searchStorage(n);
  };

  // check again if exist in acc or storage
  if (searchAccOrStorage(m_svs->getDataName(nid, s))) {
    NDN_LOG_TRACE("Already fetched " << m_svs->getDataName(nid, s));
    return;
  }

  NDN_LOG_TRACE("Trying getting data " << m_svs->getDataName(nid, s));
  m_svs->fetchData(nid, s, [this, acc, searchAccOrStorage] (const ndn::Data& data) {
    NDN_LOG_DEBUG("Getting data " << data.getName());

    NDN_LOG_TRACE("[" << data.getName() << "]: Printing acc...");
    for (auto& d : *acc) {
      NDN_LOG_TRACE("     " << d.getName());
    }

    if (!searchAccOrStorage(data.getName())) acc->push_front(data);

    // has to parse the content at this time
    Record record(data.getName(), data.getContent());
    if (record.isGenesis()) {
      NDN_LOG_INFO(data.getName() << " is an new genesis record");
    }

    // check the pointer first
    bool allExist = true;
    for (auto& p : record.getPointers()) {
      if (searchAccOrStorage(p)) {
        // then we don't need to care
        NDN_LOG_TRACE("[" << data.getName() << "]: Pointed record " << p << " already exists");
      }
      else {
        allExist = false;
        NDN_LOG_TRACE("[" << data.getName() << "]: Discover " << p);
        // we need to fetch it
        auto tuple = parseDataName(p);
        recursiveFetcher(std::get<0>(tuple), std::get<1>(tuple), acc);
      }
    }

    if (allExist) {
      // clean the acc
      for (auto& d : *acc) {
        m_storageIntf.adder(d.getName(), d.wireEncode());
        NDN_LOG_TRACE("Yielding " << d.getName());
        m_yieldCb(Record(d.getName(), d.getContent()));
      }
      acc->clear();
    }
  });
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