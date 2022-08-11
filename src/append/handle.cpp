#include "append/handle.hpp"

namespace cledger::append {

Handle::~Handle()
{
  for (auto& handle : m_interestFilterHandles) {
    handle.cancel();
  }
  for (auto& handle : m_registeredPrefixHandles) {
    handle.unregister();
  }
}

Handle&
Handle::handlePrefix(const ndn::RegisteredPrefixHandle& prefix)
{
  m_registeredPrefixHandles.push_back(prefix);
  return *this;
}
  
Handle&
Handle::handleFilter(const ndn::InterestFilterHandle& filter)
{
  m_interestFilterHandles.push_back(filter);
  return *this;
}

} // namespace cledger::append
