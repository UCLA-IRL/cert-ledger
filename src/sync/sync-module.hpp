#ifndef CLEDGER_SYNC_MODULE_HPP
#define CLEDGER_SYNC_MODULE_HPP

#include "record.hpp"
#include "storage/ledger-storage.hpp"
#include "sync/ledger-svs.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <ndn-svs/svsync-base.hpp>
#include <ndn-svs/svspubsub.hpp>

namespace cledger::sync {

using ndn::svs::SVSyncBase;
using ndn::svs::NodeID;
using ndn::svs::SeqNo;
using ndn::svs::SecurityOptions;
using ndn::svs::UpdateCallback;
using ndn::svs::MissingDataInfo;
using ndn::svs::DataStore;
using ndn::svs::SVSPubSub;
using ndn::svs::SVSPubSubOptions;

using YieldRecordCallback = std::function<void(const Record&)>;




class SyncModule
{
public:
  const ssize_t MAX_RETRIES = 3;

  explicit
  SyncModule(const SyncOptions &options, const SecurityOptions& secOps, ndn::Face& face,
             storage::Interface storageIntf, const YieldRecordCallback& yield);

  Name
  publishRecord(Record& record);

  Name
  getNextName();

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:

  SyncOptions m_syncOptions;
  SecurityOptions m_secOptions;
  ndn::Face& m_face;

  std::shared_ptr<SVSPubSub> m_ps;
  storage::Interface m_storageIntf;
  YieldRecordCallback m_yieldCb;

  SeqNo m_seqNo = 0;
};

} // namespace cledger::sync

#endif // CLEDGER_SYNC_MODULE_HPP