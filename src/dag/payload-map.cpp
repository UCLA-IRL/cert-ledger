#include "dag/payload-map.hpp"

namespace cledger::dag {

static const std::string mapNameHeader = "/32=PayloadMap";

enum : uint32_t {
  TLV_PAYLOAD_MAP_TYPE= 381,
  TLV_PAYLOAD_MAP_TO = 382,
};

Name
toMapName(const span<const uint8_t>& payload)
{
  auto hash = ndn::util::Sha256::computeDigest(payload);
  return Name(mapNameHeader).appendParametersSha256Digest(
    make_span(static_cast<const uint8_t*>(hash->data()), hash->size())
  );
}

Block
encodePayloadMap(PayloadMap& map)
{
  Block block(TLV_PAYLOAD_MAP_TYPE);
  block.push_back(map.mapName.wireEncode());
  block.push_back(ndn::makeNestedBlock(TLV_PAYLOAD_MAP_TO, map.mapTo));
  block.encode();
  return block;
}

PayloadMap
decodePayloadMap(Block& block)
{
  PayloadMap map;
  block.parse();
  if (block.type() != TLV_PAYLOAD_MAP_TYPE) {
    NDN_THROW(std::runtime_error("TLV Type is incorrect"));
  }
  for (const auto &item : block.elements()) {
    switch (item.type()) {
      case ndn::tlv::Name:
        map.mapName = Name(item);
        break;
      case TLV_PAYLOAD_MAP_TO:
        map.mapTo = Name(item.blockFromValue());
        break;
      default:
        if (ndn::tlv::isCriticalType(item.type())) {
          NDN_THROW(std::runtime_error("Unrecognized TLV Type: " + std::to_string(item.type())));
        }
        else {
          //ignore
        }
        break;
    }
  }
  return map;
}

std::ostream&
operator<<(std::ostream& os, const PayloadMap& map)
{
  os << "Payload Map Name: " << map.mapName << "\n";
 os << "   Map to: " << map.mapTo << "\n";
  return os;
}
} // cledger::dag