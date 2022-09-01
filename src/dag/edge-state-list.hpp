#ifndef CLEDGER_DAG_EDGE_STATE_LIST_HPP
#define CLEDGER_DAG_EDGE_STATE_LIST_HPP

#include "edge-state.hpp"
namespace cledger::dag {

struct EdgeStateList
{
  Name listName;
  uint32_t key;
  std::set<Name> value;

  Name nextList;
};

Name
toStateListName(uint32_t key);

uint32_t
fromStateListName(const Name& stateListName);

Name
getStateListNull();

Block
encodeEdgeStateList(EdgeStateList& stateList);

EdgeStateList
decodeEdgeStateList(Block& block);

std::ostream&
operator<<(std::ostream& os, const EdgeStateList& stateList);

} // namespace cledger::dag

#endif // CLEDGER_DAG_EDGE_STATE_LIST_HPP