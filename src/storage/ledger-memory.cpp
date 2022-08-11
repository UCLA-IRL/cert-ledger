#include "ledger-memory.hpp"

namespace cledger {
namespace ledger {

const std::string LedgerMemory::STORAGE_TYPE = "ledger-storage-memory";
CLEDGER_REGISTER_LEDGER_STORAGE(LedgerMemory);

LedgerMemory::LedgerMemory(const Name& lederName, const std::string& path)
  : LedgerStorage()
{
}

void
LedgerMemory::addData(const Data& data)
{
  Name name = data.getName();
  auto search = m_list.find(name);
  if (search != m_list.end()) {
    NDN_THROW(std::runtime_error("Data for " + name.toUri() + " already exists"));
  }
  m_list.insert(std::make_pair(name, data));
}

Data
LedgerMemory::getData(const Name& name)
{
  auto search = m_list.find(name);
  if (search == m_list.end()) {
    NDN_THROW(std::runtime_error("Data for " + name.toUri() + " does not exists"));
  }
  return search->second;
}

void
LedgerMemory::deleteData(const Name& name)
{
  auto search = m_list.find(name);
  if (search == m_list.end()) {
    NDN_THROW(std::runtime_error("Data for " + name.toUri() + " does not exists"));
  }
  m_list.erase(search);
}

} // namespace ledger
} // namespace cledger
