#include "dag/edge-state.hpp"

namespace cledger::dag {

static const std::string stateNameHeader = "/32=EdgeState";

enum : uint32_t {
  TLV_EDGE_STATE_TYPE= 311,
  TLV_EDGE_STATE_DESCENDANTS = 312
};

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

Block
encodeEdgeState(EdgeState& state)
{
  Block block(TLV_EDGE_STATE_TYPE);
  block.push_back(state.stateName.wireEncode());
  block.push_back(*state.record.prepareContent());

  Buffer nameBuffer;
  for (auto& d : state.descendants) {
    auto b = d.wireEncode();
    nameBuffer.insert(nameBuffer.end(), b.begin(), b.end());
  }

  if (state.descendants.size() > 0) {
    block.push_back(ndn::makeBinaryBlock(TLV_EDGE_STATE_DESCENDANTS, 
      span<const uint8_t>(nameBuffer.data(), nameBuffer.size())));
  }
  block.encode();
  return block;
}

EdgeState
decodeEdgeState(Block& block)
{
  EdgeState state;
  block.parse();
  if (block.type() != TLV_EDGE_STATE_TYPE) {
    NDN_THROW(std::runtime_error("TLV Type is incorrect"));
  }
  for (const auto &item : block.elements()) {
    switch (item.type()) {
      case ndn::tlv::Name:
        state.stateName = Name(item);
        break;
      case ndn::tlv::Content:
        state.record = Record(fromStateName(state.stateName), item);
        break;
      case TLV_EDGE_STATE_DESCENDANTS:
        item.parse();
        for (const auto& ptr : item.elements()) {
          state.descendants.insert(Name(ptr));
        }
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
  return state;
}

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