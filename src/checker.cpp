#include "checker.hpp"
#include "record.hpp"
#include "error.hpp"
#include <ndn-cxx/security/signing-helpers.hpp>
namespace cledger::checker {

NDN_LOG_INIT(cledger.checker);

Checker::Checker(ndn::Face& face, ndn::security::Validator& validator)
  : m_face(face)
  , m_validator(validator)
{
}

void
Checker::doCheck(const Name ledgerPrefix, const Data& data,
                 const onNackCallback onNack, 
                 const onDataCallback onData, 
                 const onFailureCallback onFailure)
{
  auto state = std::make_shared<CheckerState>(data, onNack, onData, onFailure);
  dispatchInterest(state, ledgerPrefix);
}

void
Checker::dispatchInterest(const std::shared_ptr<CheckerState>& checkerState,
                          const Name& ledgerPrefix)
{
  if (checkerState->exhaustRetries()) {
    return checkerState->onFailure(Error(Error::Code::TIMEOUT, "Running out of retries"));
  }

  m_face.expressInterest(*checkerState->makeInterest(ledgerPrefix),
    [this, checkerState] (auto&&, auto& data) {
      // naming conventiion check
      m_validator.validate(data,
        [this, checkerState, data] (const Data&) {
          NDN_LOG_DEBUG("Data conforms to trust schema");
          return onValidationSuccess(checkerState, data);
        },
        [this, checkerState, data] (const Data&, const ndn::security::ValidationError& error) {
          NDN_LOG_ERROR("Error authenticating data: " << error);
          return onValidationFailure(checkerState, error);
        }
      );
    },
    [checkerState] (auto& i, auto&&) {
      return checkerState->onFailure(Error(Error::Code::NACK, i.getName().toUri()));
    },
    [this, checkerState, ledgerPrefix] (const auto&) {
       return dispatchInterest(checkerState, ledgerPrefix);
    }
  );
}

void
Checker::onValidationSuccess(const std::shared_ptr<CheckerState>& checkerState, const Data& data)
{
  Name dataName = data.getName();
  if (dataName.get(-1).isTimestamp() && dataName.get(-2).toUri() == "data") {
    return checkerState->onData(data);
  }
  if (Nack::isValidName(dataName)) {
    Nack nack;
    nack.fromData(data);
    return checkerState->onNack(nack);
  }
  else {
    return checkerState->onFailure(Error(Error::Code::PROTO_SPECIFIC, "Uncognized data format")); 
  }
}

void
Checker::onValidationFailure(const std::shared_ptr<CheckerState>& checkerState, const ndn::security::ValidationError& error)
{
  return checkerState->onFailure(Error(Error::Code::VALIDATION_ERROR, error.getInfo()));
}
} // namespace cledger::checker
