#ifndef CLEDGER_CHECKER_HPP
#define CLEDGER_CHECKER_HPP

#include "checker-state.hpp"
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-config.hpp>

namespace cledger::checker {

class Checker : boost::noncopyable
{
public:
  explicit
  Checker(ndn::Face& face, ndn::security::Validator& validator);

  void
  doCheck(const Name ledgerPrefix, const Data& data,
          const onNackCallback onNack, 
          const onDataCallback onData, 
          const onFailureCallback onFailure);

private:
  void
  dispatchInterest(const std::shared_ptr<CheckerState>& checkerState,
                   const Name& ledgerPrefix);
  void
  onValidationSuccess(const std::shared_ptr<CheckerState>& checkerState, const Data& data);

  void
  onValidationFailure(const std::shared_ptr<CheckerState>& checkerState, const ndn::security::ValidationError& error);
  ndn::Face& m_face;
  ndn::security::Validator& m_validator;
};

} // namespace cledger::checker

#endif // CLEDGER_CHECKER_HPP
