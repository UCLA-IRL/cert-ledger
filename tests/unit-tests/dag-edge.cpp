#include "dag/edge.hpp"
#include "test-common.hpp"

namespace cledger::tests {

using ndn::util::DummyClientFace;
using ndn::security::verifySignature;
using dag::EdgeManager;

BOOST_FIXTURE_TEST_SUITE(TestDagModule, IdentityManagementTimeFixture)

BOOST_AUTO_TEST_CASE(Linear)
{
  /*
   * r1 <-- r2 <-- r3 <-- r4
   */

  Record r1, r2, r3, r4;
  r1.setName(Name("/r1"));
  r1.addPointer(r1.getName());
  
  r2.setName(Name("/r2"));
  r2.addPointer(r1.getName());

  r3.setName(Name("/r3"));
  r3.addPointer(r2.getName());
   
  r4.setName(Name("/r4"));
  r4.addPointer(r3.getName());

  EdgeManager eManager;
  eManager.add(r1);
  eManager.add(r2);
  eManager.add(r3);
  eManager.add(r4);

  BOOST_CHECK_EQUAL(3, eManager.reap(1).size());
  BOOST_CHECK_EQUAL(2, eManager.reap(2).size());
  BOOST_CHECK_EQUAL(1, eManager.reap(3).size());
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
  r1.addPointer(r1.getName());
  
  r2.setName(Name("/r2"));
  r2.addPointer(r1.getName());

  r3.setName(Name("/r3"));
  r3.addPointer(r1.getName());
   
  r4.setName(Name("/r4"));
  r4.addPointer(r2.getName());
  r4.addPointer(r3.getName());

  EdgeManager eManager;
  eManager.add(r1);
  eManager.add(r2);
  eManager.add(r3);
  eManager.add(r4);

  BOOST_CHECK_EQUAL(3, eManager.reap(1).size());
  BOOST_CHECK_EQUAL(1, eManager.reap(2).size());
  BOOST_CHECK_EQUAL(1, eManager.reap(3).size());
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
  Record r1, r2, r3, r4;
  r1.setName(Name("/r1"));
  r1.addPointer(r1.getName());
  
  r2.setName(Name("/r2"));
  r2.addPointer(r1.getName());

  r3.setName(Name("/r3"));
  r3.addPointer(r1.getName());
  r3.addPointer(r2.getName());
   
  r4.setName(Name("/r4"));
  r4.addPointer(r2.getName());
  r4.addPointer(r3.getName());

  EdgeManager eManager;
  eManager.add(r1);
  eManager.add(r2);
  eManager.add(r3);
  eManager.add(r4);

  for (auto& i :  eManager.reap(2)) std::cout << i.getName();
  BOOST_CHECK_EQUAL(3, eManager.reap(1).size());
  BOOST_CHECK_EQUAL(2, eManager.reap(2).size());
  BOOST_CHECK_EQUAL(1, eManager.reap(3).size());
}

BOOST_AUTO_TEST_SUITE_END() // TestDag

} // namespace cledger::tests
