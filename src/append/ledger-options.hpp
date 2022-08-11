#ifndef CLEDGER_APPEND_LEDGER_OPTIONS_HPP
#define CLEDGER_APPEND_LEDGER_OPTIONS_HPP

#include "append/client-options.hpp"

namespace cledger::append {
using appendtlv::AppendStatus;

class LedgerOptions : boost::noncopyable
{
public:
  explicit
  LedgerOptions(const Name& topic)
  : m_topic(topic)
  {
  }

  std::shared_ptr<ClientOptions>
  praseNotification(const Interest& notification); 

  std::shared_ptr<Interest>
  makeFetcher(ClientOptions& client);

  std::shared_ptr<Data>
  makeNotificationAck(ClientOptions& client,
                      const std::list<AppendStatus>& statusList);

private:
  Name m_topic;
};

} // namespace cledger:append

#endif // CLEDGER_APPEND_LEDGER_OPTIONS_HPP
