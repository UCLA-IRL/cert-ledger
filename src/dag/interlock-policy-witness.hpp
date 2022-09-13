#ifndef CLEDGER_DAG_INTERLOCK_POLICY_WITNESS_HPP
#define CLEDGER_DAG_INTERLOCK_POLICY_WITNESS_HPP

#include "interlock-policy.hpp"
namespace cledger::dag::policy {

class InterlockPolicyWitness : public InterlockPolicy
{
public:
  InterlockPolicyWitness(const std::string& internalConfig = "");
  const static std::string POLICY_TYPE;

  std::set<Name>
  select(const EdgeState& state) override;

  uint32_t
  evaluate(const EdgeState& state) override;

  Interface
  getInterface() override;
};

} // namespace cledger::dag::policy

#endif // CLEDGER_DAG_INTERLOCK_POLICY_WITNESS_HPP