#include "sync/sync-module.hpp"
#include "test-common.hpp"
#include "svs-core-identity-time-fixture.hpp"

namespace cledger::tests {

using ndn::util::DummyClientFace;
using ndn::security::verifySignature;
using sync::SyncModule;
using sync::SyncOptions;

BOOST_FIXTURE_TEST_SUITE(TestSyncModule, SVSCoreIdentityTimeFixture)

BOOST_AUTO_TEST_CASE(SVSUpdate)
{
  // init
  m_syncPrefix = Name("/sync-group");
  m_id = Name("/node-1");
  SecurityOptions secOps(m_keyChain);
  secOps.interestSigner->signingInfo.setSigningHmacKey("dGhpcyBpcyBhIHNlY3JldCBtZXNzYWdl");

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

BOOST_AUTO_TEST_SUITE_END() // TestDag

} // namespace cledger::tests
