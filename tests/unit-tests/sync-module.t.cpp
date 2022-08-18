#include "sync/sync-module.hpp"
#include "test-common.hpp"
#include "svs-core-identity-time-fixture.hpp"

namespace cledger::tests {

using ndn::util::DummyClientFace;
using ndn::security::verifySignature;
using sync::SyncModule;
using sync::SyncOptions;

BOOST_FIXTURE_TEST_SUITE(TestSyncModule, SVSCoreIdentityTimeFixture)

BOOST_AUTO_TEST_CASE(R1R2InOrder)
{
  // init
  m_syncPrefix = Name("/sync-group");
  m_id = Name("/node-1");
  SecurityOptions secOps(m_keyChain);
  secOps.dataSigner->signingInfo.setSha256Signing();
  secOps.interestSigner->signingInfo.setSha256Signing();

  std::set<Name> buffer;
  auto existFinder = [&buffer] (const Name& n) {
    return buffer.find(n) == buffer.end()? false : true;
  };
  auto yielder = [&buffer] (const Record& r) {
    buffer.insert(r.getName());
  };
  DummyClientFace face(io, m_keyChain, {true, true});

  SyncOptions syncOps{m_syncPrefix, Name("/node-1")};
  SyncModule syncMod(syncOps, secOps, face, existFinder, yielder);
  advanceClocks(time::milliseconds(20), 60);

  // prepare an event
  SeqNo seq = 0;
  NodeID id2 = Name("/node-2");

  // learn gensis record r1
  Record r1;
  r1.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r1.addPointer(r1.getName());
  std::string str = "Dummy Genesis Record";
  r1.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str.data()), str.size()));

  updateSeqNo(seq, id2);
  auto syncInt = makeSyncInterest(secOps);
  face.receive(*syncInt);
  advanceClocks(time::milliseconds(20), 60);
  auto data = makeData(seq, id2, *r1.prepareContent(), secOps);
  face.receive(*data);
  advanceClocks(time::milliseconds(20), 60);

  // learn gensis record r2
  Record r2;
  r2.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r2.addPointer(r1.getName());
  std::string str2 = "Dummy General Record";
  r2.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));

  updateSeqNo(seq, id2);
  syncInt = makeSyncInterest(secOps);
  face.receive(*syncInt);
  advanceClocks(time::milliseconds(20), 60);
  data = makeData(seq, id2, *r2.prepareContent(), secOps);
  face.receive(*data);
  advanceClocks(time::milliseconds(20), 60);
}

BOOST_AUTO_TEST_CASE(R2Only)
{
  // init
  m_syncPrefix = Name("/sync-group");
  m_id = Name("/node-1");
  SecurityOptions secOps(m_keyChain);
  secOps.dataSigner->signingInfo.setSha256Signing();
  secOps.interestSigner->signingInfo.setSha256Signing();

  std::set<Name> buffer;
  auto existFinder = [&buffer] (const Name& n) {
    return buffer.find(n) == buffer.end()? false : true;
  };
  auto yielder = [&buffer] (const Record& r) {
    std::cout << "yield " << r.getName() << std::endl;
    buffer.insert(r.getName());
  };
  DummyClientFace face(io, m_keyChain, {true, true});

  SyncOptions syncOps{m_syncPrefix, Name("/node-1")};
  SyncModule syncMod(syncOps, secOps, face, existFinder, yielder);
  advanceClocks(time::milliseconds(20), 60);

  // prepare an event
  SeqNo seq = 0;
  NodeID id2 = Name("/node-2");

  // learn gensis record r1
  Record r1;
  r1.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r1.addPointer(r1.getName());
  std::string str = "Dummy Genesis Record";
  r1.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str.data()), str.size()));

  updateSeqNo(seq, id2);
  auto data1 = makeData(seq, id2, *r1.prepareContent(), secOps);
  advanceClocks(time::milliseconds(20), 60);

  // learn gensis record r2
  Record r2;
  r2.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r2.addPointer(r1.getName());
  std::string str2 = "Dummy General Record";
  r2.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));

  updateSeqNo(seq, id2);
  auto syncInt = makeSyncInterest(secOps);
  face.receive(*syncInt);
  advanceClocks(time::milliseconds(20), 60);
  auto data2 = makeData(seq, id2, *r2.prepareContent(), secOps);
  face.receive(*data2);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data1);
  advanceClocks(time::milliseconds(20), 60);
}

BOOST_AUTO_TEST_CASE(R3Only)
{
  // init
  m_syncPrefix = Name("/sync-group");
  m_id = Name("/node-1");
  SecurityOptions secOps(m_keyChain);
  secOps.dataSigner->signingInfo.setSha256Signing();
  secOps.interestSigner->signingInfo.setSha256Signing();

  std::set<Name> buffer;
  auto existFinder = [&buffer] (const Name& n) {
    return buffer.find(n) == buffer.end()? false : true;
  };
  auto yielder = [&buffer] (const Record& r) {
    std::cout << "yield " << r.getName() << std::endl;
    buffer.insert(r.getName());
  };
  DummyClientFace face(io, m_keyChain, {true, true});

  SyncOptions syncOps{m_syncPrefix, Name("/node-1")};
  SyncModule syncMod(syncOps, secOps, face, existFinder, yielder);
  advanceClocks(time::milliseconds(20), 60);

  // prepare an event
  SeqNo seq = 0;
  NodeID id2 = Name("/node-2");

  // learn gensis record r1
  Record r1;
  r1.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r1.addPointer(r1.getName());
  std::string str = "Dummy Genesis Record";
  r1.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str.data()), str.size()));
  auto data1 = makeData(seq, id2, *r1.prepareContent(), secOps);

  // learn gensis record r2
  Record r2;
  r2.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r2.addPointer(r1.getName());
  std::string str2 = "Dummy General Record";
  r2.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));
  auto data2 = makeData(seq, id2, *r2.prepareContent(), secOps);

  Record r3;
  r3.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r3.addPointer(r2.getName());
  r3.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));
  auto data3 = makeData(seq, id2, *r3.prepareContent(), secOps);

  updateSeqNo(seq, id2);
  auto syncInt = makeSyncInterest(secOps);
  face.receive(*syncInt);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data3);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data2);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data1);
  advanceClocks(time::milliseconds(20), 60);
}


BOOST_AUTO_TEST_CASE(Rectangular)
{
  /*
   * r1 <------------ r2
   *  ^                ^
   *  |                |
   *  |                |
   * r3 ------------> r4
   */

  // init
  m_syncPrefix = Name("/sync-group");
  m_id = Name("/node-1");
  SecurityOptions secOps(m_keyChain);
  secOps.dataSigner->signingInfo.setSha256Signing();
  secOps.interestSigner->signingInfo.setSha256Signing();

  std::set<Name> buffer;
  auto existFinder = [&buffer] (const Name& n) {
    return buffer.find(n) == buffer.end()? false : true;
  };
  auto yielder = [&buffer] (const Record& r) {
    std::cout << "yield " << r.getName() << std::endl;
    buffer.insert(r.getName());
  };
  DummyClientFace face(io, m_keyChain, {true, true});

  SyncOptions syncOps{m_syncPrefix, Name("/node-1")};
  SyncModule syncMod(syncOps, secOps, face, existFinder, yielder);
  advanceClocks(time::milliseconds(20), 60);

  // prepare an event
  SeqNo seq = 0;
  NodeID id2 = Name("/node-2");

  // learn gensis record r1
  Record r1;
  r1.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r1.addPointer(r1.getName());
  std::string str = "Dummy Genesis Record";
  r1.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str.data()), str.size()));
  auto data1 = makeData(seq, id2, *r1.prepareContent(), secOps);

  // learn gensis record r2
  Record r2;
  r2.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r2.addPointer(r1.getName());
  std::string str2 = "Dummy General Record";
  r2.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));
  auto data2 = makeData(seq, id2, *r2.prepareContent(), secOps);

  Record r3;
  r3.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r3.addPointer(r1.getName());
  r3.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));
  auto data3 = makeData(seq, id2, *r3.prepareContent(), secOps);

  Record r4;
  r4.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r4.addPointer(r3.getName());
  r4.addPointer(r2.getName());
  r4.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));
  auto data4 = makeData(seq, id2, *r4.prepareContent(), secOps);

  updateSeqNo(seq, id2);
  auto syncInt = makeSyncInterest(secOps);
  face.receive(*syncInt);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data4);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data3);
  face.receive(*data2);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data1);
  advanceClocks(time::milliseconds(20), 60);
}

BOOST_AUTO_TEST_CASE(Rectangular2)
{
  /*
   * r1 <------------ r2
   *  ^         --->   ^
   *  |      ---       |
   *  |  ----          |
   * r3 <------------ r4
   */

  // init
  m_syncPrefix = Name("/sync-group");
  m_id = Name("/node-1");
  SecurityOptions secOps(m_keyChain);
  secOps.dataSigner->signingInfo.setSha256Signing();
  secOps.interestSigner->signingInfo.setSha256Signing();

  std::set<Name> buffer;
  auto existFinder = [&buffer] (const Name& n) {
    return buffer.find(n) == buffer.end()? false : true;
  };
  auto yielder = [&buffer] (const Record& r) {
    std::cout << "yield " << r.getName() << std::endl;
    buffer.insert(r.getName());
  };
  DummyClientFace face(io, m_keyChain, {true, true});

  SyncOptions syncOps{m_syncPrefix, Name("/node-1")};
  SyncModule syncMod(syncOps, secOps, face, existFinder, yielder);
  advanceClocks(time::milliseconds(20), 60);

  // prepare an event
  SeqNo seq = 0;
  NodeID id2 = Name("/node-2");

  // learn gensis record r1
  Record r1;
  r1.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r1.addPointer(r1.getName());
  std::string str = "Dummy Genesis Record";
  r1.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str.data()), str.size()));
  auto data1 = makeData(seq, id2, *r1.prepareContent(), secOps);

  // learn gensis record r2
  Record r2;
  r2.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r2.addPointer(r1.getName());
  std::string str2 = "Dummy General Record";
  r2.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));
  auto data2 = makeData(seq, id2, *r2.prepareContent(), secOps);

  Record r3;
  r3.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r3.addPointer(r1.getName());
  r3.addPointer(r2.getName());
  r3.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));
  auto data3 = makeData(seq, id2, *r3.prepareContent(), secOps);

  Record r4;
  r4.setName(Name(m_syncPrefix).append(id2).appendNumber(++seq));
  r4.addPointer(r3.getName());
  r4.addPointer(r2.getName());
  r4.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str2.data()), str2.size()));
  auto data4 = makeData(seq, id2, *r4.prepareContent(), secOps);

  updateSeqNo(seq, id2);
  auto syncInt = makeSyncInterest(secOps);
  face.receive(*syncInt);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data4);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data3);
  face.receive(*data2);
  advanceClocks(time::milliseconds(20), 60);
  face.receive(*data1);
  advanceClocks(time::milliseconds(20), 60);
}

BOOST_AUTO_TEST_SUITE_END() // TestDag

} // namespace cledger::tests
