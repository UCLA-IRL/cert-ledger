#ifndef CLEDGER_DAG_EDGE_HPP
#define CLEDGER_DAG_EDGE_HPP

#include "record.hpp"
#include "dag/interlock-policy.hpp"
namespace cledger::dag {

class EdgeManager {
public:
  EdgeManager&
  add(Record& record);

  EdgeManager&
  setInterlockPolicy(const std::shared_ptr<InterlockPolicy> policy); 

  std::list<Record>
  reap(const uint32_t threshold);

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::list<EdgeState>
  getAncestors(EdgeState state);

  EdgeState
  get(const Name& name);

  void
  update(const Name& name, EdgeState state);


  EdgeManager&
  onNewRecord(EdgeState& state);

  void
  evaluateAncestors(EdgeState& state);

  void
  evaluateWaitlist(EdgeState& state);

  std::map<Name, EdgeState> m_buffer;
  std::map<const uint32_t, std::set<Name>> m_waitlist;
  std::shared_ptr<InterlockPolicy> m_policy;
};

std::ostream&
operator<<(std::ostream& os, const EdgeState& state);

} // namespace cledger::dag

#endif // CLEDGER_DAG_EDGE_HPP