#include "ledger-module.hpp"
#include "svs-core-identity-time-fixture.hpp"
#include "test-common.hpp"

namespace cledger::tests {

using ndn::DummyClientFace;
using ledger::LedgerModule;

BOOST_FIXTURE_TEST_SUITE(TestLedgerModule, SVSCoreIdentityTimeFixture)

BOOST_AUTO_TEST_CASE(Single)
{
  /*
   * anchor <-- client1 <-- client2 <-- client3
   */

  auto anchorId = addIdentity(Name("/ndn/site1"));
  auto clientId1 = addSubCertificate(Name("/ndn/site1/instance1"), anchorId);
  auto clientId2 = addSubCertificate(Name("/ndn/site1/instance2"), anchorId);
  auto clientId3 = addSubCertificate(Name("/ndn/site1/instance3"), anchorId);

  DummyClientFace face(io, m_keyChain, {true, true});
  LedgerModule ledger(face, m_keyChain, "tests/unit-tests/config-files/config-ledger-1");
  ledger.afterValidation(anchorId.getDefaultKey().getDefaultCertificate());
  ledger.afterValidation(clientId1.getDefaultKey().getDefaultCertificate());
  ledger.afterValidation(clientId2.getDefaultKey().getDefaultCertificate());
  ledger.afterValidation(clientId3.getDefaultKey().getDefaultCertificate());
}

BOOST_AUTO_TEST_CASE(Merged)
{
  /*
   *  anchor <-- client1 <-- client2  
                                    |-- client6
   * client3 <-- client4 <-- client5
   */

  auto anchorId = addIdentity(Name("/ndn/site1"));
  auto clientId1 = addSubCertificate(Name("/ndn/site1/instance1"), anchorId);
  auto clientId2 = addSubCertificate(Name("/ndn/site1/instance2"), anchorId);
  auto clientId3 = addSubCertificate(Name("/ndn/site1/instance3"), anchorId);
  auto clientId4 = addSubCertificate(Name("/ndn/site1/instance4"), anchorId);
  auto clientId5 = addSubCertificate(Name("/ndn/site1/instance5"), anchorId);
  auto clientId6 = addSubCertificate(Name("/ndn/site1/instance6"), anchorId);

  sync::SecurityOptions secOps(m_keyChain);
  secOps.interestSigner->signingInfo = ndn::security::SigningInfo("hmac-sha256:z4MZh0VOKfqkLBuIm55CaGB8jt5fgvWJCDC/vbCwZKM=");
  secOps.dataSigner->signingInfo = ndn::security::SigningInfo("id:/ndn/site1/instance1");

  DummyClientFace face1(io, m_keyChain, {true, true});
  LedgerModule ledger1(face1, m_keyChain, "tests/unit-tests/config-files/config-ledger-1");
  // branch 2
  SeqNo seq = 0;
  m_syncPrefix = Name("/ndn/site1");
  m_id = Name("/instance2");

  Record r1;
  r1.setName(Name(m_syncPrefix).append(m_id).appendNumber(++seq));
  r1.setType(tlv::GENESIS_RECORD);
  r1.addPointer(r1.getName());
  auto certTlv = clientId3.getDefaultKey().getDefaultCertificate().wireEncode();
  r1.setPayload(make_span<const uint8_t>(certTlv.data(), certTlv.size()));
  auto data1 = makeData(seq, m_id, *r1.prepareContent(), secOps);

  Record r2;
  r1.setName(Name(m_syncPrefix).append(m_id).appendNumber(++seq));
  r2.addPointer(r1.getName());
  certTlv = clientId4.getDefaultKey().getDefaultCertificate().wireEncode();
  r2.setPayload(make_span<const uint8_t>(certTlv.data(), certTlv.size()));
  auto data2 = makeData(seq, m_id, *r1.prepareContent(), secOps);

  Record r3;
  r1.setName(Name(m_syncPrefix).append(m_id).appendNumber(++seq));
  r3.addPointer(r1.getName());
  certTlv = clientId5.getDefaultKey().getDefaultCertificate().wireEncode();
  r3.setPayload(make_span<const uint8_t>(certTlv.data(), certTlv.size()));
  auto data3 = makeData(seq, m_id, *r2.prepareContent(), secOps);

  // branch 1
  ledger1.afterValidation(anchorId.getDefaultKey().getDefaultCertificate());
  ledger1.afterValidation(clientId1.getDefaultKey().getDefaultCertificate());
  ledger1.afterValidation(clientId2.getDefaultKey().getDefaultCertificate());
  ledger1.afterValidation(clientId3.getDefaultKey().getDefaultCertificate());

  // merge
  advanceClocks(time::milliseconds(20), 60);
  updateSeqNo(seq, m_id);
  auto syncInt = makeSyncInterest(secOps);
  face1.receive(*syncInt);
  advanceClocks(time::milliseconds(20), 60);
  face1.receive(*data3);
  advanceClocks(time::milliseconds(20), 60);
  face1.receive(*data2);
  advanceClocks(time::milliseconds(20), 60);
  face1.receive(*data1);
  advanceClocks(time::milliseconds(20), 60);

  ledger1.afterValidation(clientId6.getDefaultKey().getDefaultCertificate());
}

BOOST_AUTO_TEST_SUITE_END() // TestLedgerModule

} // namespace cledger::tests
