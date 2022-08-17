#include "svs-core-fixture.hpp"

namespace cledger::tests {
using ndn::security::SigningInfo;

SVSCoreFixture::SVSCoreFixture()
{}

SVSCoreFixture::~SVSCoreFixture()
{}

void
SVSCoreFixture::updateSeqNo(const SeqNo& seq, const NodeID& nid)
{
  m_vv.set(nid, seq);
}

std::shared_ptr<Interest>
SVSCoreFixture::makeSyncInterest(const SecurityOptions& secOps)
{
  Name syncName(m_syncPrefix);
  syncName.append(Name::Component(m_vv.encode()));

  auto interest = std::make_shared<Interest>(syncName);
  // Add parameters digest
  interest->setApplicationParameters(span<const uint8_t>{'0'});
  interest->setInterestLifetime(time::milliseconds(1000));
  interest->setCanBePrefix(true);
  interest->setMustBeFresh(true);

  switch (secOps.interestSigner->signingInfo.getSignerType())
  {
    case SigningInfo::SIGNER_TYPE_NULL:
      break;

    // case SigningInfo::SIGNER_TYPE_HMAC:
    //   m_keyChainMem.sign(interest, m_securityOptions.interestSigner->signingInfo);
    //   break;

    default:
      secOps.interestSigner->sign(*interest);
      break;
  }
  return interest;
}

std::shared_ptr<Data>
SVSCoreFixture::makeData(const SeqNo& seq, const NodeID& nid, const Block& content, const SecurityOptions& secOps)
{
  Name dataName = Name(m_syncPrefix).append(nid).appendNumber(seq);
  auto data = std::make_shared<Data>(dataName);
  data->setContent(content);
  data->setFreshnessPeriod(1_s);
  secOps.dataSigner->sign(*data);
  return data;
}

SeqNo
SVSCoreFixture::getSeqNo(const NodeID& nid) const
{
  return m_vv.get(nid);  
}

} // namespace cledger::tests
