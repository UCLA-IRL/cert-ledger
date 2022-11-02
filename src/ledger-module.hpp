#ifndef CLEDGER_LEDGER_MODULE_HPP
#define CLEDGER_LEDGER_MODULE_HPP

#include "ledger-config.hpp"
#include "nack.hpp"

#include "append/handle.hpp"
#include "append/ledger.hpp"

#include "storage/ledger-storage.hpp"
#include "sync/sync-module.hpp"
#include "dag/dag-module.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator-config.hpp>

namespace cledger::ledger {
using appendtlv::AppendStatus;

class LedgerModule : boost::noncopyable
{
public:
  LedgerModule(ndn::Face& face, ndn::KeyChain& keyChain, const std::string& configPath, time::milliseconds replyPeriod = 3600_s);

  const std::unique_ptr<storage::LedgerStorage>&
  getLedgerStorage()
  {
    return m_storage;
  }

  LedgerConfig&
  getLedgerConf()
  {
    return m_config;
  }

  void
  onQuery(const Interest& query);

  void
  publishReply();

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:

  AppendStatus onDataSubmission(const Data& data);

  void
  registerPrefix();

  void
  afterValidation(const Data& data);

  void
  onRegisterFailed(const std::string& reason);

  void
  addPayloadMap(const span<const uint8_t>& payload, const Name& mapTo);

  Name
  getPayloadMap(const span<const uint8_t>& payload);

  void
  sendNack(const Name& name);

  void
  replyOrSendNack(const Name& name);

  void
  sendResponse(const Name& name, const Block& block, bool realtime = false);

  void
  dagHarvest();

  void
  updateStatesTracker(const Name& stateName, bool interlocked = false);

  void
  refreshReplyTimer();

  ndn::Face& m_face;
  LedgerConfig m_config;
  Scheduler m_scheduler{m_face.getIoService()};
  Name m_instancePrefix;
  ndn::KeyChain& m_keyChain;
  ndn::ValidatorConfig m_validator{m_face};

  // CA facing
  std::unique_ptr<append::Ledger> m_appendCt;
  append::Handle m_handle;

  // storage backend
  std::unique_ptr<storage::LedgerStorage> m_storage;

  // sync module
  std::unique_ptr<sync::SyncModule> m_sync;
  sync::SyncOptions m_syncOps;
  sync::SecurityOptions m_secOps{m_keyChain};

  // dag module
  std::unique_ptr<dag::policy::InterlockPolicy> m_policy;
  std::unique_ptr<dag::DagModule> m_dag;
  // this shouldn't keep growing, so it's safe to put into the memory
  std::set<Name> m_repliedRecords;

  // reply management
  time::milliseconds m_replyPeriod;
  ndn::scheduler::EventId m_replyEvent;
};

} // namespace cledger::ledger

#endif // CLEDGER_LEDGER_MODULE_HPP
