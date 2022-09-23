#ifndef CLEDGER_CHECKER_HPP
#define CLEDGER_CHECKER_HPP

#include "checker-state.hpp"
#include "cledger-common.hpp"
#include "util/segment/consumer.hpp"
#include "util/segment/pipeline-interests-fixed.hpp"

#include <ndn-cxx/security/validator-config.hpp>

namespace cledger::checker {

class Checker : boost::noncopyable
{
public:
  explicit
  Checker(ndn::Face& face, ndn::security::Validator& validator);

  void
  doCheck(const Name ledgerPrefix, const Data& data,
          const onSuccessCallback onSuccess, 
          const onFailureCallback onFailure);

private:
  void
  dispatchInterest(const std::shared_ptr<CheckerState>& checkerState,
                   const Name& ledgerPrefix);
  void
  onValidationSuccess(const std::shared_ptr<CheckerState>& checkerState, const Data& data);

  void
  onValidationFailure(const std::shared_ptr<CheckerState>& checkerState, const ndn::security::ValidationError& error);

  void
  decodeContent(std::vector<Data>& dataVector, const Block& content);

  ndn::Face& m_face;
  ndn::security::Validator& m_validator;
  util::segment::Options m_options;
  std::shared_ptr<util::segment::PipelineInterestsFixed> m_pipeline;
  std::shared_ptr<util::segment::Consumer> m_consumer;
};

} // namespace cledger::checker

#endif // CLEDGER_CHECKER_HPP
