#include "dag/interlock-policy-descendants.hpp"

namespace cledger::dag {

InterlockPolicyDescendants::InterlockPolicyDescendants()
  : InterlockPolicy()
{}

uint32_t
InterlockPolicyDescendants::evaluate(const EdgeState& state)
{
  return state.descendants.size();
}
} // namespace cledger::dag