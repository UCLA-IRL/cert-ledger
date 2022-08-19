#include "dag/edge-state.hpp"

namespace cledger::dag {

static const std::string stateNameHeader = "/32=EdgeState";

Name
toStateName(const Name& recordName)
{
  return Name(stateNameHeader).append(recordName);
}

Name
fromStateName(const Name& stateName)
{
  return stateName.getSubName(1);
}

// Block
// encodeEdgeState(const EdgeState& state)
// {
  
// }

std::ostream&
operator<<(std::ostream& os, const EdgeState& state)
{
  os << "Edge State Name: " << state.stateName << "\n";
  for (auto& p : state.record.getPointers()) {
    os << "   Pointer: " << p << "\n";
  }
  for (auto& d : state.descendants) {
    os << "   Descendant: " << d << "\n";
  }
  return os;
}
} // cledger::dag