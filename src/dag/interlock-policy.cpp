#include "dag/interlock-policy.hpp"

namespace cledger::dag {

std::ostream&
operator<<(std::ostream& os, const EdgeState& state)
{
  os << "Edge State Name: " << state.name << "\n";
  for (auto& p : state.pointers) {
    os << "   Pointer: " << p << "\n";
  }
  for (auto& d : state.descendants) {
    os << "   Descendant: " << d << "\n";
  }
  return os;
}

} // namespace cledger::dag