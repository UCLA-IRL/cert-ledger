#include "record.hpp"

namespace cledger {

Record::Record()
  : m_type(tlv::GENERIC_RECORD)
{
}

Record::Record(const Record& record)
  : m_name(record.getName())
  , m_type(record.getType())
  , m_pointers(record.getPointers())
  , m_payload(record.getPayload())
{
}

Record::Record(const Name& name, const Block& content)
{
  m_name = name;
  content.parse();
  for (const auto &item : content.elements()) {
    switch (item.type()) {
      case tlv::TLV_RECORD_TYPE:
        m_type = readNonNegativeInteger(item);
        break;
      case tlv::TLV_RECORD_PAYLOAD:
        m_payload = make_span<const uint8_t>(item.value(), item.value_size());
        break;
      case tlv::TLV_RECORD_POINTER:
        item.parse();
        for (const auto& ptr : item.elements()) {
          m_pointers.push_back(Name(ptr));
        }
        break;
      default:
        if (ndn::tlv::isCriticalType(item.type())) {
          NDN_THROW(Error("Unrecognized TLV Type: " + std::to_string(item.type())));
        }
        else {
          //ignore
        }
        break;
    }
  }
}

Record&
Record::setName(const Name& name)
{
  m_name = name;
  return *this;
}

Record&
Record::setType(const RecordType type)
{
  m_type = type;
  return *this;
}

Record&
Record::setPointers(const std::list<Name> name)
{
  m_pointers = name;
  return *this;
}

Record&
Record::addPointer(const Name& name)
{
  m_pointers.push_back(name);
  return *this;
}

Record&
Record::setPayload(const span<const uint8_t>& payload)
{
  m_payload = payload;
  return *this;
}

std::shared_ptr<Block>
Record::prepareContent()
{
  auto content = std::make_shared<Block>(ndn::tlv::Content);
  std::vector<uint8_t> ptrsEnc;

  for (auto& ptr : m_pointers) {
    auto b = ptr.wireEncode();
    ptrsEnc.insert(ptrsEnc.end(), b.begin(), b.end());
  }
  content->push_back(ndn::makeNonNegativeIntegerBlock(tlv::TLV_RECORD_TYPE, m_type));
  content->push_back(ndn::makeBinaryBlock(tlv::TLV_RECORD_POINTER, 
                                          span<const uint8_t>(ptrsEnc.data(), ptrsEnc.size())));
  content->push_back(ndn::makeBinaryBlock(tlv::TLV_RECORD_PAYLOAD, m_payload));
  return content;
}

bool
Record::isGenesis()
{
  return m_type == tlv::GENESIS_RECORD? true : false;
}

} // namespace cledger