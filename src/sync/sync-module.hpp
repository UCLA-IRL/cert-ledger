#ifndef CLEDGER_SYNC_MODULE_HPP
#define CLEDGER_SYNC_MODULE_HPP

#include "record.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <ndn-svs/svsync-base.hpp>

namespace cledger::sync {
NDN_LOG_INIT(cledger.sync);

using ndn::svs::SVSyncBase;
using ndn::svs::NodeID;
using ndn::svs::SeqNo;
using ndn::svs::SecurityOptions;
using ndn::svs::UpdateCallback;
using ndn::svs::MissingDataInfo;
using ndn::svs::DataStore;

using GetLocalRecord = std::function<Record(const Name&)>;
using ExistLocalRecord = std::function<bool(const Name&)>;
using YieldRecordCallback = std::function<void(std::map<Name, Record>&)>;

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
  SyncModule(const SyncOptions &options, ndn::Face& face,
            const ExistLocalRecord& exist, const YieldRecordCallback& yield);

  std::tuple<NodeID, SeqNo>
  parseDataName(const Name& name);

  void
  run()
  {
    std::thread thread_svs([this] { m_face.processEvents(); });
    while (true) {
    }

    thread_svs.join();
  }

  void
  onMissingData(const std::vector<MissingDataInfo>& vectors);

  void
  recursiveFetcher(const NodeID& nid, const SeqNo& s, std::map<Name, Record>& acc);

  void
  publishRecord(Record& record);

public:
  SyncOptions m_syncOptions;
  ndn::Face& m_face;
  ndn::KeyChain m_keyChain;
  ndn::security::SigningInfo m_signingInfo;
  SecurityOptions m_secOptions;

  std::shared_ptr<LedgerSVS> m_svs;
  ExistLocalRecord m_existCb;
  YieldRecordCallback m_yieldCb;
};

} // namespace cledger::sync

#endif // CLEDGER_SYNC_MODULE_HPP