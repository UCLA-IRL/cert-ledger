#ifndef CLEDGER_DAG_EDGE_STATE_HPP
#define CLEDGER_DAG_EDGE_STATE_HPP

#include "record.hpp"
namespace cledger::dag {

struct EdgeState
{
  enum Status {
    INITIALIZED = 0,
    LOADED = 2,
    INTERLOCKED = 3,
  };
  Name stateName;

  Record record;
  std::set<Name> descendants;

  Status status;

  time::system_clock::time_point created;
  time::system_clock::time_point interlocked;
};

Name
toStateName(const Name& recordName);

Name
fromStateName(const Name& stateName);

Block
encodeEdgeState(EdgeState& state);

EdgeState
decodeEdgeState(Block& block);

std::ostream&
operator<<(std::ostream& os, const EdgeState& state);

} // namespace cledger::dag

#endif // CLEDGER_DAG_EDGE_STATE_HPP