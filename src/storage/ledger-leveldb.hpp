#ifndef CLEDGER_STORAGE_LEVELDB_HPP
#define CLEDGER_STORAGE_LEVELDB_HPP

#include "ledger-storage.hpp"
#include <leveldb/db.h>

namespace cledger {
namespace storage {

class LedgerLevelDB : public LedgerStorage
{
public:
  LedgerLevelDB(const Name& ledgerName = Name(), const std::string& path = "");
  ~LedgerLevelDB();
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
  leveldb::DB* db;
  leveldb::Options options;
};

} // namespace storage
} // namespace cledger

#endif // NDNREVOKE_CT_DB_HPP
