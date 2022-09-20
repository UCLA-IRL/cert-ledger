#include "dag/edge-state-list.hpp"

namespace cledger::dag {

enum : uint32_t {
  TLV_EDGE_STATE_LIST_TYPE = 351,
  TLV_EDGE_STATE_LIST_NAME = 352,
  TLV_EDGE_STATE_LIST_KEY = 353,
  TLV_EDGE_STATE_LIST_VALUE = 354,
  TLV_EDGE_STATE_LIST_NEXT = 355,
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

Name
getStateListNull()
{
  return Name(stateListNull);
}

Block
encodeEdgeStateList(EdgeStateList& stateList)
{
  Block block(TLV_EDGE_STATE_LIST_TYPE);
  block.push_back(ndn::makeNestedBlock(TLV_EDGE_STATE_LIST_NAME, stateList.listName));
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
  block.push_back(ndn::makeNestedBlock(TLV_EDGE_STATE_LIST_NEXT, stateList.nextList));
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
      case TLV_EDGE_STATE_LIST_NAME:
        stateList.listName.wireDecode(Block(make_span<const uint8_t>(item.value(), item.value_size())));
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
      case TLV_EDGE_STATE_LIST_NEXT:
        stateList.nextList.wireDecode(Block(make_span<const uint8_t>(item.value(), item.value_size())));
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