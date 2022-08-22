#include "dag/interlock-policy-descendants.hpp"

namespace cledger::dag::policy {

const std::string InterlockPolicyDescendants::POLICY_TYPE = "policy-descendants";
CLEDGER_REGISTER_INTERLOCK_POLICY(InterlockPolicyDescendants);

InterlockPolicyDescendants::InterlockPolicyDescendants(const std::string& internalConfig)
  : InterlockPolicy()
{
}

uint32_t
InterlockPolicyDescendants::evaluate(const EdgeState& state)
{
  return state.descendants.size();
}

Interface
InterlockPolicyDescendants::getInterface()
{
  Interface intf;
  intf.evaluater = std::bind(&InterlockPolicyDescendants::evaluate, this, _1);
  return intf;  
}
} // namespace cledger::dag::policy