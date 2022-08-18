#ifndef CLEDGER_DAG_INTERLOCK_POLICY_DESCENDANTS_HPP
#define CLEDGER_DAG_INTERLOCK_POLICY_DESCENDANTS_HPP

#include "interlock-policy.hpp"
namespace cledger::dag {

class InterlockPolicyDescendants : public InterlockPolicy
{
public:
  InterlockPolicyDescendants();

  uint32_t
  evaluate(const EdgeState& state) override;
};

} // namespace cledger::dag

#endif // CLEDGER_DAG_INTERLOCK_POLICY_DESCENDANTS_HPP