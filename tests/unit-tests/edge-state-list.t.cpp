#include "dag/edge-state-list.hpp"
#include "test-common.hpp"

namespace cledger::tests {

using dag::EdgeStateList;
using dag::encodeEdgeStateList;
using dag::decodeEdgeStateList;

BOOST_FIXTURE_TEST_SUITE(TestEdgeStateList, IdentityManagementTimeFixture)

BOOST_AUTO_TEST_CASE(Encoding)
{
  EdgeStateList input;
  input.listName = Name("/32=EdgeStateList/l1");
  input.key = 1;
  input.value.insert(Name("/32=EdgeState/r1"));

  Block block = encodeEdgeStateList(input);
  EdgeStateList output = decodeEdgeStateList(block);
  BOOST_CHECK_EQUAL(input.listName, output.listName);
  BOOST_CHECK_EQUAL(input.value.size(), output.value.size());
}

BOOST_AUTO_TEST_SUITE_END() // TestEdgeStateList

} // namespace cledger::tests