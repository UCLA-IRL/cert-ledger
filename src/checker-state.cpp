#include "checker-state.hpp"
#include "record.hpp"
#include <ndn-cxx/security/signing-helpers.hpp>
namespace cledger::checker {

CheckerState::CheckerState(ndn::Face& face,
                           const Data& data,
                           const onNackCallback onNack, 
                           const onDataCallback onData, 
                           const onFailureCallback onFailure)
  : m_face(face)
  , m_data(data)
  , m_nCb(onNack)
  , m_dCb(onData)
  , m_fCb(onFailure)
{
}

std::shared_ptr<Interest>
CheckerState::makeInterest(const Name& ledgerPrefix)
{
  Name interestName = m_data.getName();
  interestName.appendKeyword("record");
  auto interest = std::make_shared<Interest>(interestName);
  interest->setMustBeFresh(true);
  interest->setCanBePrefix(true);
  interest->setForwardingHint({ledgerPrefix});
  return interest;
}

} // namespace cledger::checker
