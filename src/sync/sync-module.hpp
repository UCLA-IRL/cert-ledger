#ifndef CLEDGER_SYNC_MODULE_HPP
#define CLEDGER_SYNC_MODULE_HPP

#include "record.hpp"
#include "storage/ledger-storage.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <ndn-svs/svsync-base.hpp>

namespace cledger::sync {

using ndn::svs::SVSyncBase;
using ndn::svs::NodeID;
using ndn::svs::SeqNo;
using ndn::svs::SecurityOptions;
using ndn::svs::UpdateCallback;
using ndn::svs::MissingDataInfo;
using ndn::svs::DataStore;

using YieldRecordCallback = std::function<void(const Record&)>;

struct SyncOptions
{
  Name prefix;
  Name id;
};

class LedgerSVS : public SVSyncBase
{
public:
  LedgerSVS(const Name& syncPrefix,
            const Name& nodePrefix,
            ndn::Face& face,
            const UpdateCallback& updateCallback,
            const SecurityOptions& securityOptions = SecurityOptions::DEFAULT,
            std::shared_ptr<DataStore> dataStore = DEFAULT_DATASTORE);

  Name
  getDataName(const NodeID& nid, const SeqNo& seqNo) override;
};

class SyncModule
{
public:
  explicit
  SyncModule(const SyncOptions &options, const SecurityOptions& secOps, ndn::Face& face,
             storage::Interface storageIntf, const YieldRecordCallback& yield);

  std::tuple<NodeID, SeqNo>
  parseDataName(const Name& name);

  void
  onMissingData(const std::vector<MissingDataInfo>& vectors);

  void
  recursiveFetcher(const NodeID& nid, const SeqNo& s, std::shared_ptr<std::list<Data>> acc);

  void
  publishRecord(Record& record);

public:
  SyncOptions m_syncOptions;
  SecurityOptions m_secOptions;
  ndn::Face& m_face;

  std::shared_ptr<LedgerSVS> m_svs;
  storage::Interface m_storageIntf;
  YieldRecordCallback m_yieldCb;
};

} // namespace cledger::sync

#endif // CLEDGER_SYNC_MODULE_HPP