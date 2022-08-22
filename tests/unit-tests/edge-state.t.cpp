#include "dag/edge-state.hpp"
#include "test-common.hpp"

namespace cledger::tests {

using dag::EdgeState;
using dag::encodeEdgeState;
using dag::decodeEdgeState;

BOOST_FIXTURE_TEST_SUITE(TestEdgeState, IdentityManagementTimeFixture)

BOOST_AUTO_TEST_CASE(Encoding)
{
  EdgeState input;
  input.stateName = Name("/32=EdgeState/r1");
  input.descendants.insert(Name("/32=EdgeState/r2"));

  Block block = encodeEdgeState(input);
  EdgeState output = decodeEdgeState(block);
  BOOST_CHECK_EQUAL(input.stateName, output.stateName);
  BOOST_CHECK_EQUAL(input.descendants.size(), output.descendants.size());
}

BOOST_AUTO_TEST_SUITE_END() // TestEdgeState

} // namespace cledger::tests