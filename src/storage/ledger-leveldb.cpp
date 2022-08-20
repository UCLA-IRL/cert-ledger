#include "ledger-leveldb.hpp"

namespace cledger {
namespace storage {

const std::string LedgerLevelDB::STORAGE_TYPE = "storage-leveldb";
CLEDGER_REGISTER_STORAGE(LedgerLevelDB);

LedgerLevelDB::LedgerLevelDB(const Name& ledgerName, const std::string& path)
  : LedgerStorage()
{
  // open and initialize the leveldb database
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &db);
  if(!status.ok()){
    NDN_THROW(std::runtime_error("leveldb cannot be opened/created."));
  }
}

LedgerLevelDB::~LedgerLevelDB()
{
  delete db;
}

void
LedgerLevelDB::addBlock(const Name& name, const Block& block)
{
  std::string value;
  leveldb::Status s=db->Get(leveldb::ReadOptions(), name.toUri(), &value);
  if (s.ok()){
    NDN_THROW(std::runtime_error("Data for " + name.toUri() + " already exists"));
  }
  std::string data_str=std::string(reinterpret_cast<const char*>(block.wire()), block.size());
  s = db->Put(leveldb::WriteOptions(), name.toUri(), data_str);
  if (!s.ok()){
    NDN_THROW(std::runtime_error("DB cannot append new entry: "+ name.toUri()));
  }
}

Block
LedgerLevelDB::getBlock(const Name& name)
{
  std::string data_str;
  Data ret;
  leveldb::Status s=db->Get(leveldb::ReadOptions(), name.toUri(), &data_str);
  if (!s.ok()){
    NDN_THROW(std::runtime_error("Data for " + name.toUri() + " does not exists"));
  }
  return Block({reinterpret_cast<const uint8_t*>(data_str.data()), data_str.size()});
}

void
LedgerLevelDB::deleteBlock(const Name& name)
{
  std::string data_str;
  leveldb::Status s=db->Get(leveldb::ReadOptions(), name.toUri(), &data_str);
  if (!s.ok()){
    NDN_THROW(std::runtime_error("Data for " + name.toUri() + " does not exists"));
  }
  s=db->Delete(leveldb::WriteOptions(), name.toUri());
  if (!s.ok()){
    NDN_THROW(std::runtime_error("DB cannot delete the entry: " + name.toUri()));
  }
}

} // namespace storage
} // namespace cledger
