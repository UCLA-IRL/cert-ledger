#include "dag/edge-state-list.hpp"

namespace cledger::dag {

static const std::string stateListNameHeader = "/32=EdgeStateList";

enum : uint32_t {
  TLV_EDGE_STATE_LIST_TYPE = 351,
  TLV_EDGE_STATE_LIST_KEY = 352,
  TLV_EDGE_STATE_LIST_VALUE = 353,
};


Name
toStateListName(uint32_t key)
{
  return Name(stateListNameHeader).appendNumber(key);
}

uint32_t
fromStateListName(const Name& stateListName)
{
  return stateListName.get(-1).toNumber();
}

Block
encodeEdgeStateList(EdgeStateList& stateList)
{
  Block block(TLV_EDGE_STATE_LIST_TYPE);
  block.push_back(stateList.listName.wireEncode());
  block.push_back(ndn::makeNonNegativeIntegerBlock(TLV_EDGE_STATE_LIST_KEY, stateList.key));

  Buffer nameBuffer;
  for (auto& n : stateList.value) {
    auto nTlv = n.wireEncode();
    nameBuffer.insert(nameBuffer.end(), nTlv.begin(), nTlv.end());
  }

  if (stateList.value.size() > 0) {
    block.push_back(ndn::makeBinaryBlock(TLV_EDGE_STATE_LIST_VALUE, 
      span<const uint8_t>(nameBuffer.data(), nameBuffer.size())));
  }
  block.encode();
  return block;
}

EdgeStateList
decodeEdgeStateList(Block& block)
{
  EdgeStateList stateList;
  block.parse();
  if (block.type() != TLV_EDGE_STATE_LIST_TYPE) {
    NDN_THROW(std::runtime_error("TLV Type is incorrect"));
  }
  for (const auto &item : block.elements()) {
    switch (item.type()) {
      case ndn::tlv::Name:
        stateList.listName = Name(item);
        break;
      case TLV_EDGE_STATE_LIST_KEY:
        stateList.key = ndn::readNonNegativeInteger(item);
        break;
      case TLV_EDGE_STATE_LIST_VALUE:
        item.parse();
        for (const auto& stateName : item.elements()) {
          stateList.value.insert(Name(stateName));
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
  return stateList;
}

std::ostream&
operator<<(std::ostream& os, const EdgeStateList& stateList)
{
  os << "Edge State List Name: " << stateList.listName << "\n";
  os << "   List Key: " << stateList.key << "\n";
  for (auto& n : stateList.value) {
    os << "   List Value Item: " << n << "\n";
  }
  return os;
}
} // cledger::dag