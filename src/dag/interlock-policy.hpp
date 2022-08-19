#ifndef CLEDGER_DAG_INTERLOCK_POLICY_HPP
#define CLEDGER_DAG_INTERLOCK_POLICY_HPP

#include "record.hpp"
#include "dag/edge-state.hpp"
namespace cledger::dag {

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