#ifndef CLEDGER_DAG_PAYLOAD_MAP_HPP
#define CLEDGER_DAG_PAYLOAD_MAP_HPP

#include "cledger-common.hpp"

namespace cledger::dag {

struct PayloadMap
{
  Name mapName;
  Name mapTo;
};

Name
toMapName(const span<const uint8_t>& payload);

Block
encodePayloadMap(PayloadMap& map);

PayloadMap
decodePayloadMap(Block& block);

std::ostream&
operator<<(std::ostream& os, const PayloadMap& map);

} // namespace cledger::dag

#endif // CLEDGER_DAG_EDGE_STATE_HPP