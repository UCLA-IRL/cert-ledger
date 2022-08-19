#ifndef CLEDGER_RECORD_HPP
#define CLEDGER_RECORD_HPP

#include "cledger-common.hpp"

namespace cledger {

using RecordType = uint32_t;

class Record
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    using ndn::tlv::Error::Error;
  };

  Record();

  Record(const Name& name, const Block& content);
  
  std::shared_ptr<Block>
  prepareContent();

  const Name
  getName() const
  {
    return m_name;
  }

  RecordType
  getType() const
  {
    return m_type;
  }

  const std::list<Name>
  getPointers() const
  {
    return m_pointers;
  }

  const span<const uint8_t>&
  getPayload() const
  {
    return m_payload;
  }
  
  Record&
  setName(const Name& name);

  Record&
  setType(const RecordType type);

  Record&
  setPointers(const std::list<Name> name);

  Record&
  addPointer(const Name& name);

  Record&
  setPayload(const span<const uint8_t>& payload);

  bool
  isGenesis();

private:
  Name m_name;
  RecordType m_type;
  std::list<Name> m_pointers;
  span<const uint8_t> m_payload;
};

std::ostream&
operator<<(std::ostream& os, const Record& record);

class GenesisRecord : public Record {
  public:
    GenesisRecord(int number);
};

} // namespace cledger
#endif // CLEDGER_RECORD_HPP
