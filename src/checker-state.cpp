#include "checker-state.hpp"
#include "record.hpp"
#include <ndn-cxx/security/signing-helpers.hpp>
namespace cledger::checker {

CheckerState::CheckerState(const Data& data,
                           const onSuccessCallback onSuccess, 
                           const onFailureCallback onFailure)
  : m_data(data)
  , m_sCb(onSuccess)
  , m_fCb(onFailure)
{
}

std::shared_ptr<Interest>
CheckerState::makeInterest(const Name& ledgerPrefix)
{
  Name interestName = m_data.getName();
  interestName.set(-4, Name::Component("RECORD"));
  auto interest = std::make_shared<Interest>(interestName);
  interest->setMustBeFresh(true);
  interest->setCanBePrefix(true);
  interest->setForwardingHint({ledgerPrefix});
  return interest;
}

} // namespace cledger::checker
