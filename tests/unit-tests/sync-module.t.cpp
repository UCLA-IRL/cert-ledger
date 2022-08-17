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


    // using ExistLocalRecord = std::function<bool(const Name&)>;
    // using YieldRecordCallback = std::function<void(std::list<Record>&)>;
  
  std::set<Name> buffer;
  auto existFinder = [buffer] (const Name& n) {
    return buffer.find(n) == buffer.end()? false : true;
  };
  auto yielder = [] (const Record&) {
    std::cout << "yield\n";
  };
  DummyClientFace face(io, m_keyChain, {true, true});

  SyncOptions syncOps{m_syncPrefix, Name("/node-1")};
  SyncModule syncMod(syncOps, secOps, face, existFinder, yielder);
  advanceClocks(time::milliseconds(20), 60);

  // prepare an event
  SeqNo seq = 1;
  NodeID id2 = Name("/node-2");
  updateSeqNo(seq, id2);
  auto syncInt1 = makeSyncInterest(secOps);
  face.receive(*syncInt1);
  advanceClocks(time::milliseconds(20), 60);

  // prepare a gensis record
  Record record;
  record.setName(Name(m_syncPrefix).append(id2).appendNumber(seq));
  record.addPointer(record.getName());
  std::string str = "Dummy Genesis Record";
  record.setPayload(span<const uint8_t>(reinterpret_cast<const uint8_t*>(str.data()), str.size()));

  auto data = makeData(seq, id2, *record.prepareContent(), secOps);
  face.receive(*data);
  advanceClocks(time::milliseconds(20), 60);
}

BOOST_AUTO_TEST_SUITE_END() // TestDag

} // namespace cledger::tests
