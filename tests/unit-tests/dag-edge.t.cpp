#include "dag/dag-module.hpp"
#include "storage/ledger-memory.hpp"
#include "dag/interlock-policy-descendants.hpp"
#include "test-common.hpp"

namespace cledger::tests {

using ndn::DummyClientFace;
using ndn::security::verifySignature;
using dag::DagModule;

BOOST_FIXTURE_TEST_SUITE(TestDagModule, IdentityManagementTimeFixture)

BOOST_AUTO_TEST_CASE(Linear)
{
  /*
   * r1 <-- r2 <-- r3 <-- r4
   */

  Record r1, r2, r3, r4;
  r1.setName(Name("/r1"));
  r1.setType(tlv::GENESIS_RECORD);
  r1.addPointer(r1.getName());
  
  r2.setName(Name("/r2"));
  r2.addPointer(r1.getName());

  r3.setName(Name("/r3"));
  r3.addPointer(r2.getName());
   
  r4.setName(Name("/r4"));
  r4.addPointer(r3.getName());

  auto storage = storage::LedgerStorage::createLedgerStorage("storage-memory", "/test/ledger", "");
  auto policy = dag::policy::InterlockPolicy::createInterlockPolicy("policy-descendants", "");
  DagModule eManager(storage->getInterface(), policy->getInterface());
  eManager.add(r1);
  eManager.add(r2);
  eManager.add(r3);
  eManager.add(r4);

  BOOST_CHECK_EQUAL(3, eManager.harvestAbove(1).size());
  BOOST_CHECK_EQUAL(2, eManager.harvestAbove(2).size());
  BOOST_CHECK_EQUAL(1, eManager.harvestAbove(3, true).size());
  BOOST_CHECK_EQUAL(0, eManager.harvestAbove(3).size());
  // for (auto& s : eManager.m_buffer) {
  //   std::cerr << s.second;
  // }
}

BOOST_AUTO_TEST_CASE(Rectangular1)
{
  /*
   * r1 <------------ r2
   *  ^                ^
   *  |                |
   *  |                |
   * r3 <------------ r4
   */
  Record r1, r2, r3, r4;
  r1.setName(Name("/r1"));
  r1.setType(tlv::GENESIS_RECORD);
  r1.addPointer(r1.getName());
  
  r2.setName(Name("/r2"));
  r2.addPointer(r1.getName());

  r3.setName(Name("/r3"));
  r3.addPointer(r1.getName());
   
  r4.setName(Name("/r4"));
  r4.addPointer(r2.getName());
  r4.addPointer(r3.getName());

  auto storage = storage::LedgerStorage::createLedgerStorage("storage-memory", "/test/ledger", "");
  auto policy = dag::policy::InterlockPolicy::createInterlockPolicy("policy-descendants", "");
  DagModule eManager(storage->getInterface(), policy->getInterface());
  eManager.add(r1);
  eManager.add(r2);
  eManager.add(r3);
  eManager.add(r4);

  BOOST_CHECK_EQUAL(3, eManager.harvestAbove(1).size());
  BOOST_CHECK_EQUAL(1, eManager.harvestAbove(2).size());
  BOOST_CHECK_EQUAL(1, eManager.harvestAbove(3, true).size());
  BOOST_CHECK_EQUAL(0, eManager.harvestAbove(3).size());
  // for (auto& s : eManager.m_buffer) {
  //   std::cerr << s.second;
  // }
}

BOOST_AUTO_TEST_CASE(Rectangular2)
{
  /*
   * r1 <------------ r2
   *  ^                ^
   *  |                |
   *  |                |
   * r3 ------------> r4
   */
  Record r1, r2, r3, r4;
  r1.setName(Name("/r1"));
  r1.setType(tlv::GENESIS_RECORD);
  r1.addPointer(r1.getName());
  
  r2.setName(Name("/r2"));
  r2.addPointer(r1.getName());
   
  r4.setName(Name("/r4"));
  r4.addPointer(r2.getName());

  r3.setName(Name("/r3"));
  r3.addPointer(r1.getName());
  r3.addPointer(r4.getName());

  auto storage = storage::LedgerStorage::createLedgerStorage("storage-memory", "/test/ledger", "");
  auto policy = dag::policy::InterlockPolicy::createInterlockPolicy("policy-descendants", "");
  DagModule eManager(storage->getInterface(), policy->getInterface());
  eManager.add(r1);
  eManager.add(r2);
  eManager.add(r4);
  eManager.add(r3);

  BOOST_CHECK_EQUAL(3, eManager.harvestAbove(1).size());
  BOOST_CHECK_EQUAL(2, eManager.harvestAbove(2).size());
  BOOST_CHECK_EQUAL(1, eManager.harvestAbove(3, true).size());
  BOOST_CHECK_EQUAL(0, eManager.harvestAbove(3).size());
  // for (auto& s : eManager.m_buffer) {
  //   std::cerr << s.second;
  // }
}

BOOST_AUTO_TEST_CASE(Rectangular3)
{
  /*
   * r1 <------------ r2
   *  ^         --->   ^
   *  |      ---       |
   *  |  ----          |
   * r3 <------------ r4
   */
  Record r1, r2, r3, r4;
  r1.setName(Name("/r1"));
  r1.setType(tlv::GENESIS_RECORD);
  r1.addPointer(r1.getName());
  
  r2.setName(Name("/r2"));
  r2.addPointer(r1.getName());

  r3.setName(Name("/r3"));
  r3.addPointer(r1.getName());
  r3.addPointer(r2.getName());
   
  r4.setName(Name("/r4"));
  r4.addPointer(r2.getName());
  r4.addPointer(r3.getName());

  auto storage = storage::LedgerStorage::createLedgerStorage("storage-memory", "/test/ledger", "");
  auto policy = dag::policy::InterlockPolicy::createInterlockPolicy("policy-descendants", "");
  DagModule eManager(storage->getInterface(), policy->getInterface());
  eManager.add(r1);
  eManager.add(r2);
  eManager.add(r3);
  eManager.add(r4);

  BOOST_CHECK_EQUAL(3, eManager.harvestAbove(1).size());
  BOOST_CHECK_EQUAL(2, eManager.harvestAbove(2).size());
  BOOST_CHECK_EQUAL(1, eManager.harvestAbove(3, true).size());
  BOOST_CHECK_EQUAL(0, eManager.harvestAbove(3).size());
  // for (auto& s : eManager.m_buffer) {
  //   std::cerr << s.second;
  // }
}

BOOST_AUTO_TEST_SUITE_END() // TestDag

} // namespace cledger::tests
