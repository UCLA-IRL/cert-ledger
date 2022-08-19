#ifndef CLEDGER_DAG_EDGE_STATE_HPP
#define CLEDGER_DAG_EDGE_STATE_HPP

#include "record.hpp"
namespace cledger::dag {

struct EdgeState
{
  enum Status {
    INITIALIZED = 0,
    LOADED = 2,
  };
  Name stateName;
//   std::list<Name> pointers;
//   span<const uint8_t> payload;

  Record record;
  std::set<Name> descendants;

  Status status;
};

Name
toStateName(const Name& recordName);

Name
fromStateName(const Name& stateName);

Block
encodeEdgeState(const EdgeState& state);

std::ostream&
operator<<(std::ostream& os, const EdgeState& state);

} // namespace cledger::dag

#endif // CLEDGER_DAG_EDGE_STATE_HPP