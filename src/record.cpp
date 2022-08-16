#include "record.hpp"

namespace cledger {

Record::Record()
{
}

Record::Record(const Record& record)
  : m_name(record.getName())
  , m_pointers(record.getPointers())
  , m_payload(record.getPayload())
{
}

Record::Record(const Block& block)
  : Record(Data(block))
{
}

Record::Record(const Data& data)
{
  Record record;
  record.fromData(data);
  m_name = record.getName();
  m_pointers = record.getPointers();
  m_payload = record.getPayload();
}

Record&
Record::setName(const Name& name)
{
  m_name = name;
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

void
Record::fromData(const Data& data)
{
  m_name = data.getName();
  Block content = data.getContent();

  content.parse();
  for (const auto &item : content.elements()) {
    switch (item.type()) {
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

std::shared_ptr<Data>
Record::prepareData()
{
  auto data = std::make_shared<Data>(m_name);
  Block content(ndn::tlv::Content);
  Block pointers;

  for (auto& ptr : m_pointers) {
    pointers.push_back(ptr.wireEncode());
  }
  content.push_back(ndn::makeBinaryBlock(tlv::TLV_RECORD_POINTER, pointers));
  content.push_back(ndn::makeBinaryBlock(tlv::TLV_RECORD_PAYLOAD, m_payload));
  data->setContent(content);
  return data; 
}

} // namespace cledger