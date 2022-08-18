#ifndef CLEDGER_DAG_INTERLOCK_POLICY_HPP
#define CLEDGER_DAG_INTERLOCK_POLICY_HPP

#include "record.hpp"
namespace cledger::dag {

struct EdgeState
{
  enum Status {
    INITIALIZED = 0,
    LOADED = 2,
  };
  Name name;
  std::list<Name> pointers;
  span<const uint8_t> payload;
  std::set<Name> descendants;

  Status status;
};

std::ostream&
operator<<(std::ostream& os, const EdgeState& state);

class InterlockPolicy {
public:
  virtual
  ~InterlockPolicy() = default;

  virtual uint32_t
  evaluate(const EdgeState& state) = 0;
};

} // namespace cledger::dag

#endif // CLEDGER_DAG_INTERLOCK_POLICY_HPP