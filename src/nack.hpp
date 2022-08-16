#ifndef CLEDGER_NACK_HPP
#define CLEDGER_NACK_HPP

#include "cledger-common.hpp"

namespace cledger {

class Nack : boost::noncopyable
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    using ndn::tlv::Error::Error;
  };

  Nack();

  explicit
  Nack(const Block& block);

  explicit
  Nack(const Data& data);

  void
  fromData(const Data& data);

  const Name
  getName() const
  {
    return m_name;
  }

  const time::milliseconds
  getTimestamp() const
  {
    auto timestamp = m_name.get(TIMESTAMP_OFFSET).toTimestamp();
    return time::toUnixTimestamp(timestamp);   
  }

  std::shared_ptr<Data>
  prepareData(const Name dataName, const time::milliseconds timestamp);

  static bool isValidName(const Name name);

  // /<data-prefix>/nack/<timestamp>
  static const ssize_t TIMESTAMP_OFFSET;
  static const ssize_t NACK_OFFSET;

protected:
  Name m_name;
};

std::ostream&
operator<<(std::ostream& os, const Nack& nack);

} // namespace cledger

#endif // CLEDGER_NACK_HPP
