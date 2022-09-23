#include "checker.hpp"
#include "record.hpp"
#include "error.hpp"
#include "util/validate-multiple.hpp"
#include <ndn-cxx/security/signing-helpers.hpp>
namespace cledger::checker {

#ifdef CLEDGER_WITH_BENCHMARK
NDN_LOG_INIT(cledger.benchmark.checker);
#else
NDN_LOG_INIT(cledger.checker);
#endif

Checker::Checker(ndn::Face& face, ndn::security::Validator& validator)
  : m_face(face)
  , m_validator(validator)
  , m_pipeline(std::make_shared<util::segment::PipelineInterestsFixed>(m_face, m_options))
{
}

void
Checker::doCheck(const Name ledgerPrefix, const Data& data,
                 const onSuccessCallback onSuccess, 
                 const onFailureCallback onFailure)
{
  auto state = std::make_shared<CheckerState>(data, onSuccess, onFailure);
  dispatchInterest(state, ledgerPrefix);
}

void
Checker::dispatchInterest(const std::shared_ptr<CheckerState>& checkerState,
                          const Name& ledgerPrefix)
{
  if (checkerState->exhaustRetries()) {
    return checkerState->onFailure(Error(Error::Code::TIMEOUT, "Running out of retries"));
  }

  auto interest = checkerState->makeInterest(ledgerPrefix);
  NDN_LOG_INFO("Expressing Interest " << *interest << " ...");
  m_face.expressInterest(*interest,
    [this, checkerState] (auto&&, auto& data) {
      // naming convention check
      m_validator.validate(data,
        [this, checkerState, data] (const Data& d) {
        NDN_LOG_INFO("Data " << d.getName() << " conforms to trust schema");
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
  if (dataName.get(-2).isVersion() && dataName.get(-3).toUri() == "data") {
    // run segment consumer
    m_consumer = std::make_shared<util::segment::Consumer>(m_validator, 
      [this, checkerState] (auto& block) {
        std::vector<Data> dataVector;
        decodeContent(dataVector, block);
        NDN_LOG_DEBUG(dataVector.size() << " Data were encapuslated");
        util::validateMultipleData(m_validator, dataVector,
          [checkerState, block] (auto& d) { 
            checkerState->onSuccess(block);
          },
          [checkerState] (auto&&, auto& e) { 
            checkerState->onFailure(Error(Error::Code::VALIDATION_ERROR, e.getInfo()));
          }
        );
      }
    );
    m_consumer->run(dataName.getPrefix(-1), m_pipeline);
  }
  else if (Nack::isValidName(dataName)) {
    Nack nack;
    nack.fromData(data);
    return checkerState->onFailure(Error(Error::Code::NACK, "Application Layer Nack " + data.getName().toUri())); 
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

void
Checker::decodeContent(std::vector<Data>& dataVector, const Block& content)
{
  content.parse();
  for (const auto &item : content.elements()) {
    switch (item.type()) {
      case ndn::tlv::Data:
        dataVector.push_back(Data(item));
        break;
      default:
        if (ndn::tlv::isCriticalType(item.type())) {
          NDN_THROW(Error(Error::Code::PROTO_SPECIFIC, "Unrecognized TLV Type" + std::to_string(item.type())));
        }
        else {
          //ignore
        }
        break;
    }
  }
}
} // namespace cledger::checker
