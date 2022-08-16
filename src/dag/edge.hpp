#ifndef CLEDGER_DAG_EDGE_HPP
#define CLEDGER_DAG_EDGE_HPP

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

class EdgeManager {
public:
  EdgeManager&
  add(Record& record);

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
  evaluateWaitlist(EdgeState& state);

  std::map<Name, EdgeState> m_buffer;
  std::map<const uint32_t, std::set<Name>> m_waitlist;
};

std::ostream&
operator<<(std::ostream& os, const EdgeState& state);

} // namespace cledger::dag

#endif // CLEDGER_DAG_EDGE_HPP