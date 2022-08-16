#include "nack.hpp"

namespace cledger {

const ssize_t Nack::TIMESTAMP_OFFSET = -1;
const ssize_t Nack::NACK_OFFSET = -2;

Nack::Nack()
{
}

Nack::Nack(const Block& block)
  : Nack(Data(block))
{
}

Nack::Nack(const Data& data)
{
  Nack nack;
  nack.fromData(data);
  m_name = nack.getName();
}


void
Nack::fromData(const Data& data)
{
  m_name = data.getName();
}

std::shared_ptr<Data>
Nack::prepareData(const Name dataName, time::milliseconds timestamp)
{
  Name name(dataName);
  name.append("nack");
  name.appendTimestamp(ndn::time::fromUnixTimestamp(timestamp));
  auto data = std::make_shared<Data>(name);
  data->setContentType(ndn::tlv::ContentType_Nack);
  return data;
}

bool
Nack::isValidName(const Name name)
{
  return name.get(NACK_OFFSET) == Name::Component("nack") &&
         name.get(TIMESTAMP_OFFSET).isTimestamp();
}

std::ostream&
operator<<(std::ostream& os, const Nack& nack)
{
  os << "Nacked Data Name: "
     << nack.getName().getPrefix(Nack::NACK_OFFSET) << "\n"
     << "Nack Timestamp: ["
     << time::toString(time::fromUnixTimestamp(nack.getTimestamp())) << "]\n";
  return os;
}

} // namespace cledger