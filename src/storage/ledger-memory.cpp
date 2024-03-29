#include "ledger-memory.hpp"

namespace cledger {
namespace storage {

const std::string LedgerMemory::STORAGE_TYPE = "storage-memory";
CLEDGER_REGISTER_STORAGE(LedgerMemory);

LedgerMemory::LedgerMemory(const Name& lederName, const std::string& path)
  : LedgerStorage()
{
}

void
LedgerMemory::addBlock(const Name& name, const Block& block)
{
  auto search = m_list.find(name);
  if (search != m_list.end()) {
    NDN_THROW(std::runtime_error("Block for " + name.toUri() + " already exists"));
  }
  m_list.insert(std::make_pair(name, block));
}

Block
LedgerMemory::getBlock(const Name& name)
{
  auto search = m_list.find(name);
  if (search == m_list.end()) {
    NDN_THROW(std::runtime_error("Block for " + name.toUri() + " does not exists"));
  }
  return search->second;
}

void
LedgerMemory::deleteBlock(const Name& name)
{
  auto search = m_list.find(name);
  if (search == m_list.end()) {
    NDN_THROW(std::runtime_error("Block for " + name.toUri() + " does not exists"));
  }
  m_list.erase(search);
}

Interface
LedgerMemory::getInterface()
{
  Interface intf;
  intf.adder = std::bind(&LedgerMemory::addBlock, this, _1, _2);
  intf.getter = std::bind(&LedgerMemory::getBlock, this, _1);
  intf.deleter = std::bind(&LedgerMemory::deleteBlock, this, _1);
  return intf;
}

} // namespace storage
} // namespace cledger
