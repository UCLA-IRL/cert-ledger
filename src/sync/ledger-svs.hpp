#ifndef CLEDGER_SYNC_LEDGER_SVS_HPP
#define CLEDGER_SYNC_LEDGER_SVS_HPP

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
using ndn::svs::DataStore;

struct SyncOptions
{
  Name prefix;
  Name id;
};

class LedgerSVSDataStore : public DataStore
{
public:
  LedgerSVSDataStore(storage::Interface storageIntf);

  std::shared_ptr<const Data>
  find(const Interest& interest) override;

  void
  insert(const Data& data) override;

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  storage::Interface m_storageIntf;
};

} // namespace cledger::sync

#endif // CLEDGER_SYNC_LEDGER_SVS_HPP