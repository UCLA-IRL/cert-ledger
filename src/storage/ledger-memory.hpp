#ifndef CLEDGER_STORAGE_MEMORY_HPP
#define CLEDGER_STORAGE_MEMORY_HPP

#include "ledger-storage.hpp"

namespace cledger {
namespace storage {

class LedgerMemory : public LedgerStorage
{
public:
  LedgerMemory(const Name& ledgerName = Name(), const std::string& path = "");
  const static std::string STORAGE_TYPE;

public:
  void
  addBlock(const Name& name, const Block& block) override;

  Block
  getBlock(const Name& name) override;

  void
  deleteBlock(const Name& name) override;

  Interface
  getInterface() override;

private:
  std::map<Name, Block> m_list;
};

} // namespace storage
} // namespace cledger

#endif // NDNREVOKE_CT_MEMORY_HPP
