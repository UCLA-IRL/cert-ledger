#ifndef CLEDGER_APPEND_HANDLE_HPP
#define CLEDGER_APPEND_HANDLE_HPP

#include "append/append-common.hpp"
namespace cledger::append {

class Handle : boost::noncopyable
{
public:
  explicit
  Handle()
  {
  }

  ~Handle();

  Handle&
  handlePrefix(const ndn::RegisteredPrefixHandle& prefix);
  
  Handle&
  handleFilter(const ndn::InterestFilterHandle& filter);

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::list<ndn::RegisteredPrefixHandle> m_registeredPrefixHandles;
  std::list<ndn::InterestFilterHandle> m_interestFilterHandles;
};

} // namespace cledger::append

#endif // CLEDGER_APPEND_HANDLE_HPP
