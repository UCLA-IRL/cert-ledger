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
  addData(const Data& data) override;

  Data
  getData(const Name& name) override;

  void
  deleteData(const Name& name) override;

private:
  std::map<Name, Data> m_list;
};

} // namespace storage
} // namespace cledger

#endif // NDNREVOKE_CT_MEMORY_HPP
