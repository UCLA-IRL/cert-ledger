#include "append/client-options.hpp"

namespace cledger::append {
namespace tlv = appendtlv;

std::shared_ptr<Interest>
ClientOptions::makeNotification()
{
  auto notification = std::make_shared<Interest>(Name(m_topic).append("notify"));
  // notification parameter: m_prefix, [m_forwardingHint], nonce
  Block params(ndn::tlv::ApplicationParameters);
  params.push_back(makeNestedBlock(appendtlv::AppenderPrefix, getPrefix()));
  if (!getForwardingHint().empty()) {
    params.push_back(makeNestedBlock(ndn::tlv::ForwardingHint, getForwardingHint()));
  }
  params.push_back(ndn::makeNonNegativeIntegerBlock(appendtlv::AppenderNonce, getNonce()));
  params.encode();
  notification->setApplicationParameters(params);
  return notification;
}

const Name
ClientOptions::makeInterestFilter()
{
  return Name(getPrefix()).append("msg").append(m_topic)
                          .appendNumber(getNonce());
}

std::shared_ptr<Data>
ClientOptions::makeSubmission(const std::list<Data>& dataList)
{
  // Data: /<m_prefix>/msg/<topic>/<nonce>
  Name name = Name(getPrefix()).append("msg").append(m_topic)
                               .appendNumber(getNonce())
                               .appendNumber(getNonce2());
  auto data = std::make_shared<Data>(name);
  Block content(ndn::tlv::Content);
  int dataCount = 0;
  for (auto& item : dataList) {
    dataCount++;
    content.push_back(item.wireEncode());
  }
  content.encode();
  data->setContent(content);
  return data;
}

std::list<AppendStatus>
ClientOptions::praseAck(const Data& data)
{ 
  auto content = data.getContent();
  content.parse();
  std::list<AppendStatus> statusList;
  for (const auto &item : content.elements()) {
    switch (item.type()) {
      case tlv::AppendStatusCode:
        statusList.push_back(static_cast<AppendStatus>(readNonNegativeInteger(item)));
        break;
      default:
        if (ndn::tlv::isCriticalType(item.type())) {
          NDN_THROW(std::runtime_error("Unrecognized TLV Type: " + std::to_string(item.type())));
        }
        else {
          // ignore
        }
        break;
    }
  }
  return statusList;
}
} // namespace cledger::append