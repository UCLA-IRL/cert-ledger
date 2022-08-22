#include "dag/interlock-policy.hpp"

namespace cledger::dag::policy {

std::unique_ptr<InterlockPolicy>
InterlockPolicy::createInterlockPolicy(const std::string& InterlockPolicyType, const std::string& internalConfig)
{
  InterlockPolicyFactory& factory = getFactory();
  auto i = factory.find(InterlockPolicyType);
  return i == factory.end() ? nullptr : i->second(internalConfig);
}

InterlockPolicy::InterlockPolicyFactory&
InterlockPolicy::getFactory()
{
  static InterlockPolicy::InterlockPolicyFactory factory;
  return factory;
}
}