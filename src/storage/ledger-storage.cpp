#include "ledger-storage.hpp"

namespace cledger {
namespace ledger {

std::unique_ptr<LedgerStorage>
LedgerStorage::createLedgerStorage(const std::string& ledgerStorageType, const Name& ledgerName, const std::string& path)
{
  LedgerStorageFactory& factory = getFactory();
  auto i = factory.find(ledgerStorageType);
  return i == factory.end() ? nullptr : i->second(ledgerName, path);
}

LedgerStorage::LedgerStorageFactory&
LedgerStorage::getFactory()
{
  static LedgerStorage::LedgerStorageFactory factory;
  return factory;
}

} // namespace ledger
} // namespace cledger
