#include "append/ledger-options.hpp"

namespace cledger::append {
namespace tlv = appendtlv;

std::shared_ptr<ClientOptions>
LedgerOptions::praseNotification(const Interest& notification)
{
  Name prefix;
  uint64_t nonce = appendtlv::InvalidNonce;
  Name fwHint;
  auto params = notification.getApplicationParameters();
  params.parse();
  for (const auto &item : params.elements()) {
    switch (item.type()) {
      case appendtlv::AppenderPrefix:
        prefix = Name(item.blockFromValue());
        break;
      case ndn::tlv::ForwardingHint:
        fwHint = Name(item.blockFromValue());
        break;
      case appendtlv::AppenderNonce:
        nonce = readNonNegativeInteger(item);
        break;
      default:
        if (ndn::tlv::isCriticalType(item.type())) {
          NDN_THROW(std::runtime_error("Unrecognized TLV Type: " + std::to_string(item.type())));
        }
        else {
          //ignore
        }
        break;
    }
  }
  if (fwHint.empty()) {
    return std::make_shared<ClientOptions>(prefix, m_topic, nonce,
                                           nullptr, nullptr);    
  }
  else {
    return std::make_shared<ClientOptions>(prefix, m_topic, nonce,
                                           nullptr, nullptr, fwHint);
  }
}

std::shared_ptr<Interest>
LedgerOptions::makeFetcher(ClientOptions& client)
{
  client.setNonce2(ndn::random::generateSecureWord64());
  Name name = Name(client.getPrefix()).append("msg").append(m_topic)
                                      .appendNumber(client.getNonce())
                                      .appendNumber(client.getNonce2());
  auto fetcher = std::make_shared<Interest>(name);
  auto fwHint = client.getForwardingHint();
  if (!fwHint.empty()) {
    fetcher->setForwardingHint({fwHint});
  }
  return fetcher;
}

std::shared_ptr<Data>
LedgerOptions::makeNotificationAck(ClientOptions& client,
                                   const std::list<AppendStatus>& statusList)
{
  auto notification = client.makeNotification();
  auto data = std::make_shared<Data>(notification->getName());
  // acking notification
  Block content(ndn::tlv::Content);
  for (auto& status : statusList) {
    content.push_back(ndn::makeNonNegativeIntegerBlock(tlv::AppendStatusCode, static_cast<uint64_t>(status)));
  }
  content.encode();
  data->setContent(content);
  return data;
}
} // namespace cledger::append