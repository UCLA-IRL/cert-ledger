#include "svs-core-fixture.hpp"

namespace cledger::tests {
using ndn::security::SigningInfo;

SVSCoreFixture::SVSCoreFixture(const Name& syncPrefix,
                               const SecurityOptions& securityOptions,
                               const NodeID& nid)
  : m_syncPrefix(syncPrefix)
  , m_securityOptions(securityOptions)
  , m_id(nid)
{}

void
SVSCoreFixture::updateSeqNo(const SeqNo& seq, const NodeID& nid)
{
  SeqNo prev = m_vv.get(nid);
  m_vv.set(nid, seq);
}

std::shared_ptr<Interest>
SVSCoreFixture::makeSyncInterest()
{
  Name syncName(m_syncPrefix);
  syncName.append(Name::Component(m_vv.encode()));

  auto interest = std::make_shared<Interest>(syncName);
  // Add parameters digest
  interest->setApplicationParameters(span<const uint8_t>{'0'});
  interest->setInterestLifetime(time::milliseconds(1000));
  interest->setCanBePrefix(true);
  interest->setMustBeFresh(true);

  switch (m_securityOptions.interestSigner->signingInfo.getSignerType())
  {
    case SigningInfo::SIGNER_TYPE_NULL:
      break;

    // case SigningInfo::SIGNER_TYPE_HMAC:
    //   m_keyChainMem.sign(interest, m_securityOptions.interestSigner->signingInfo);
    //   break;

    default:
      m_securityOptions.interestSigner->sign(*interest);
      break;
  }
  return interest;
}

SeqNo
SVSCoreFixture::getSeqNo(const NodeID& nid) const
{
  return m_vv.get(nid);  
}

} // namespace cledger::tests
