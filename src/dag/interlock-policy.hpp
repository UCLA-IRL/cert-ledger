#ifndef CLEDGER_DAG_INTERLOCK_POLICY_HPP
#define CLEDGER_DAG_INTERLOCK_POLICY_HPP

#include "record.hpp"
#include "dag/edge-state.hpp"
namespace cledger::dag::policy {

using Evaluater = std::function<uint32_t(const EdgeState&)>;
struct Interface {
  Evaluater evaluater;
};

class InterlockPolicy
{
public:

  virtual std::set<Name>
  select(const EdgeState& state) = 0;

  virtual uint32_t
  evaluate(const EdgeState& state) = 0;

  virtual Interface
  getInterface() = 0;

public:
  template<class InterlockPolicyType>
  static void
  registerInterlockPolicy(const std::string& interlockPolicyType = InterlockPolicyType::POLICY_TYPE)
  {
    InterlockPolicyFactory& factory = getFactory();
    factory[interlockPolicyType] = [] (const std::string& internalConfig) {
      return std::make_unique<InterlockPolicyType>(internalConfig);
    };
  }

  static std::unique_ptr<InterlockPolicy>
  createInterlockPolicy(const std::string& interlockPolicyType, const std::string& internalConfig);

  virtual
  ~InterlockPolicy() = default;

private:
  using InterlockPolicyCreateFunc = std::function<std::unique_ptr<InterlockPolicy> (const std::string&)>;
  using InterlockPolicyFactory = std::map<std::string, InterlockPolicyCreateFunc>;

  static InterlockPolicyFactory&
  getFactory();
};

#define CLEDGER_REGISTER_INTERLOCK_POLICY(C)                         \
static class Cledger ## C ## InterlockPolicyRegistrationClass        \
{                                                                \
public:                                                          \
  Cledger ## C ## InterlockPolicyRegistrationClass()                 \
  {                                                              \
    ::cledger::dag::policy::InterlockPolicy::registerInterlockPolicy<C>();          \
  }                                                              \
} g_Cledger ## C ## InterlockPolicyRegistrationVariable

} // namespace cledger::dag::policy

#endif // CLEDGER_DAG_INTERLOCK_POLICY_HPP