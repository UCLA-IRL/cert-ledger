#ifndef CLEDGER_DAG_INTERLOCK_POLICY_DESCENDANTS_HPP
#define CLEDGER_DAG_INTERLOCK_POLICY_DESCENDANTS_HPP

#include "interlock-policy.hpp"
namespace cledger::dag::policy {

class InterlockPolicyDescendants : public InterlockPolicy
{
public:
  InterlockPolicyDescendants(const std::string& internalConfig = "");
  const static std::string POLICY_TYPE;

  uint32_t
  evaluate(const EdgeState& state) override;

  Interface
  getInterface() override;
};

} // namespace cledger::dag::policy

#endif // CLEDGER_DAG_INTERLOCK_POLICY_DESCENDANTS_HPP