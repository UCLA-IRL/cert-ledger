#ifndef CLEDGER_LEDGER_MODULE_HPP
#define CLEDGER_LEDGER_MODULE_HPP

#include "storage/ledger-storage.hpp"
#include "append/handle.hpp"
#include "append/ledger.hpp"
#include "ledger-config.hpp"
#include "nack.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator-config.hpp>

namespace cledger::ledger {
using appendtlv::AppendStatus;

class LedgerModule : boost::noncopyable
{
public:
  LedgerModule(ndn::Face& face, ndn::KeyChain& keyChain, const std::string& configPath,
           const std::string& storageType = "storage-memory");

  const std::unique_ptr<storage::LedgerStorage>&
  getLedgerStorage()
  {
    return m_storage;
  }

  LedgerConfig&
  getCtConf()
  {
    return m_config;
  }

  void
  onQuery(const Interest& query);


CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:

  AppendStatus onDataSubmission(const Data& data);

  void
  registerPrefix();

  void
  onRegisterFailed(const std::string& reason);

  bool
  isValidQuery(Name queryName);

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  ndn::Face& m_face;
  LedgerConfig m_config;
  ndn::KeyChain& m_keyChain;
  ndn::ValidatorConfig m_validator{m_face};
  std::unique_ptr<append::Ledger> m_appendCt;
  std::unique_ptr<storage::LedgerStorage> m_storage;

  append::Handle m_handle;
};

} // namespace ndnrevoke::ct

#endif // NDNREVOKE_CT_MODULE_HPP
