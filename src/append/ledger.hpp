#ifndef CLEDGER_APPEND_LEDGER_HPP
#define CLEDGER_APPEND_LEDGER_HPP

#include "append/ledger-options.hpp"
#include "append/handle.hpp"

namespace cledger::append {
using appendtlv::AppendStatus;

using UpdateCallback = std::function<AppendStatus(const Data&)>;

class Ledger : boost::noncopyable
{
public:
  explicit
  Ledger(const Name& prefix, const Name& topic, ndn::Face& face, 
     ndn::KeyChain& keyChain, ndn::security::Validator& validator);

  void
  listen(const UpdateCallback& onUpdateCallback);

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void
  serveClient(std::shared_ptr<ClientOptions> client);

  void
  onValidationSuccess(const Data& data, std::shared_ptr<ClientOptions> client);

  void
  onValidationFailure(const Data& data, const ndn::security::ValidationError& error,
                      std::shared_ptr<ClientOptions> client);

  Name m_prefix;
  ndn::Face& m_face;
  Name m_topic;
  LedgerOptions m_options{m_topic};
  ssize_t m_retryCount = 0;

  UpdateCallback m_onUpdate;
  Handle m_handle;

  ndn::KeyChain& m_keyChain;
  ndn::security::Validator& m_validator;
};

} // namespace cledger::append

#endif // CLEDGER_APPEND_HANDLE_CT_STATE_HPP
