#include "append/client.hpp"
#include <ndn-cxx/security/signing-helpers.hpp>

namespace cledger::append {
namespace tlv = appendtlv;

#ifdef CLEDGER_WITH_BENCHMARK
NDN_LOG_INIT(cledger.benchmark.append);
#else
NDN_LOG_INIT(cledger.append);
#endif

Client::Client(const Name& prefix, ndn::Face& face,
               ndn::KeyChain& keyChain, ndn::security::Validator& validator)
  : m_face(face)
  , m_prefix(prefix)
  , m_keyChain(keyChain)
  , m_validator(validator)
{
}

Client::Client(const Name& prefix, ndn::Face& face,
               const Name& fwHint,
               ndn::KeyChain& keyChain, ndn::security::Validator& validator)
  : m_face(face)
  , m_prefix(prefix)
  , m_keyChain(keyChain)
  , m_validator(validator)
{
}

void
Client::dispatchNotification(const std::shared_ptr<ClientOptions>& options, const std::list<Data>& data)
{
  auto interest = options->makeNotification();
  if (options->exhaustRetries()) {
    options->onFailure(data, Error(Error::Code::TIMEOUT, interest->getName().toUri() + " not acked"));
    return;
  }
  NDN_LOG_INFO("Sending out notification " << *interest);
  m_face.expressInterest(*interest, 
    [this, options, data] (auto&&, const auto& notificationAck) {
      m_handle.unregisterFilters();
      m_validator.validate(notificationAck, 
        [this, options, data, notificationAck] (const Data&) {
          NDN_LOG_INFO("ACK conforms to trust schema");
          onValidationSuccess(options, data, notificationAck);
        },
        [this, options, data, notificationAck] (const Data&, const ndn::security::ValidationError& error) {
          onValidationFailure(options, data, error);
        });
    }, 
    [options, data] (auto& i, auto& n) {
      NDN_LOG_INFO("Notification Nack: " << n.getReason()); 
      options->onFailure(data, Error(Error::Code::NACK, i.getName().toUri()));
    },
    [this, options, data] (const auto&) { dispatchNotification(options, data);}
  );
}

uint64_t
Client::appendData(const Name& topic, const std::list<Data>& data,
                   const ClientOptions::onSuccessCallback onSuccess,
                   const ClientOptions::onFailureCallback onFailure)
{
  // sanity check
  if (topic.empty() || data.size() == 0 || 
      data.front().getName().empty()) {
    NDN_LOG_ERROR("Empty data or topic, return");
    return appendtlv::InvalidNonce;
  }

  auto options = std::make_shared<ClientOptions>(m_prefix, topic,
      ndn::random::generateSecureWord64(), onSuccess, onFailure);
  // prepare submission
  Name filterName = options->makeInterestFilter();
  auto filterId = m_face.setInterestFilter(filterName,
    [this, options, data] (auto&&, const auto& i) {
      if (options->getNonce2() > 0) {
        NDN_LOG_WARN("Already matched with " << options->getNonce2());
        return;
      }
      else {
        options->setNonce2(i.getName().get(-1).toNumber());
      }
      auto submission = options->makeSubmission(data);
      m_keyChain.sign(*submission, ndn::signingByIdentity(options->getPrefix()));
      m_face.put(*submission);
      NDN_LOG_DEBUG("Submitting " << *submission);
    }
  );
  // handle the unregsiter task in destructor
  m_handle.handleFilter(filterId);
  NDN_LOG_DEBUG("Registering filter for " << filterName);
  dispatchNotification(options, data);
  return options->getNonce();
}

void
Client::onValidationSuccess(const std::shared_ptr<ClientOptions>& options, const std::list<Data>& data, const Data& ack)
{
  auto statusList = options->praseAck(ack);
  // if all success, onSuccess; otherwise, failure
  for (auto& status: statusList) {
    if (status == tlv::AppendStatus::SUCCESS) {
      continue;
    }
    else {
      NDN_LOG_INFO("There are individual submissions failed by ledger");
    }
  }
  m_retryCount = 0;
  options->onSuccess(data, ack);
}

void
Client::onValidationFailure(const std::shared_ptr<ClientOptions>& options, const std::list<Data>& data,
                                 const ndn::security::ValidationError& error)
{
  NDN_LOG_ERROR("Error authenticating ACK: " << error);
  options->onFailure(data, Error(Error::Code::VALIDATION_ERROR, error.getInfo()));
}

} // namespace cledger::append
