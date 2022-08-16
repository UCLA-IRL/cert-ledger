#ifndef CLEDGER_RECORD_HPP
#define CLEDGER_RECORD_HPP

#include "cledger-common.hpp"

namespace cledger {

class Record : boost::noncopyable
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    using ndn::tlv::Error::Error;
  };

  Record();

  explicit
  Record(const Record& record);

  explicit
  Record(const Data& data);

  explicit
  Record(const Block& block);

  void
  fromData(const Data& data);
  
  std::shared_ptr<Data>
  prepareData();

  const Name
  getName() const
  {
    return m_name;
  }

  const Name
  getProducer() const
  {
    return m_name.getPrefix(-1);
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
  setPointers(const std::list<Name> name);

  Record&
  addPointer(const Name& name);

  Record&
  setPayload(const span<const uint8_t>& payload);

private:
  /**
   * The record-name as
   * /<producer-prefix>/RECORD/<event-name>
   */
  Name m_name;
  /**
   * The list of pointers to preceding records.
   */
  std::list<Name> m_pointers;
  /**
   * The data structure to carry the record body payloads.
   */
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
