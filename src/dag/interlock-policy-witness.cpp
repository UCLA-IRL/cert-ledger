#include "dag/interlock-policy-witness.hpp"

namespace cledger::dag::policy {

const std::string InterlockPolicyWitness::POLICY_TYPE = "policy-witness";
CLEDGER_REGISTER_INTERLOCK_POLICY(InterlockPolicyWitness);

InterlockPolicyWitness::InterlockPolicyWitness(const std::string& internalConfig)
  : InterlockPolicy()
{
}


std::set<Name>
InterlockPolicyWitness::select(const EdgeState& state)
{
  std::set<Name> witness;
  std::set<Name> ret;
  for (auto& item : state.descendants) {
    // it's the same to compare dataprefix
    auto dataPerfix = dag::fromStateName(item).getPrefix(-1);
    if (witness.find(dataPerfix) == witness.end()) {
      witness.insert(dataPerfix);
      ret.insert(item);
    }
  }
  return ret;
}

uint32_t
InterlockPolicyWitness::evaluate(const EdgeState& state)
{
  return select(state).size();
}

Interface
InterlockPolicyWitness::getInterface()
{
  Interface intf;
  intf.evaluater = std::bind(&InterlockPolicyWitness::evaluate, this, _1);
  return intf;  
}
} // namespace cledger::dag::policy