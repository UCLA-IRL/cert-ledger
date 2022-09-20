#include "ledger-leveldb.hpp"

namespace cledger {
namespace storage {

const std::string LedgerLevelDB::STORAGE_TYPE = "storage-leveldb";
CLEDGER_REGISTER_STORAGE(LedgerLevelDB);

LedgerLevelDB::LedgerLevelDB(const Name& ledgerName, const std::string& path)
  : LedgerStorage()
  , m_path(path)
{
  // open and initialize the leveldb database
  m_options.create_if_missing = true;
  // do a trial first
  leveldb::DB* db;
  leveldb::Status status = leveldb::DB::Open(m_options, path, &db);
  if(!status.ok()) {
    delete db;
    NDN_THROW(std::runtime_error("leveldb cannot be opened/created."));
  }
  delete db;
}

LedgerLevelDB::~LedgerLevelDB()
{
}

void
LedgerLevelDB::addBlock(const Name& name, const Block& block)
{
  std::string value;
  leveldb::DB* db;
  leveldb::Status status = leveldb::DB::Open(m_options, m_path, &db);
  if(!status.ok()) {
    delete db;
    NDN_THROW(std::runtime_error("leveldb cannot be opened/created."));
  }
  status = db->Get(leveldb::ReadOptions(), name.toUri(), &value);
  if (status.ok()){
    delete db;
    NDN_THROW(std::runtime_error("Block for " + name.toUri() + " already exists"));
  }
  std::string data_str = std::string(reinterpret_cast<const char*>(block.wire()), block.size());
  status = db->Put(leveldb::WriteOptions(), name.toUri(), data_str);
  if (!status.ok()){
    delete db;
    NDN_THROW(std::runtime_error("DB cannot append new block: "+ name.toUri()));
  }
  delete db;
}

Block
LedgerLevelDB::getBlock(const Name& name)
{
  std::string data_str;
  Data ret;
  leveldb::DB* db;
  leveldb::Status status = leveldb::DB::Open(m_options, m_path, &db);
  if(!status.ok()) {
    delete db;
    NDN_THROW(std::runtime_error("leveldb cannot be opened/created."));
  }
  status = db->Get(leveldb::ReadOptions(), name.toUri(), &data_str);
  if (!status.ok()){
    delete db;
    NDN_THROW(std::runtime_error("Block for " + name.toUri() + " does not exists"));
  }
  delete db;
  return Block(make_span<const uint8_t>(reinterpret_cast<const uint8_t*>(data_str.data()), data_str.size()));
  
}

void
LedgerLevelDB::deleteBlock(const Name& name)
{
  std::string data_str;
  leveldb::DB* db;
  leveldb::Status status = leveldb::DB::Open(m_options, m_path, &db);
  if(!status.ok()) {
    delete db;
    NDN_THROW(std::runtime_error("leveldb cannot be opened/created."));
  }
  status = db->Get(leveldb::ReadOptions(), name.toUri(), &data_str);
  if (!status.ok()){
    delete db;
    NDN_THROW(std::runtime_error("Block for " + name.toUri() + " does not exists"));
  }
  status = db->Delete(leveldb::WriteOptions(), name.toUri());
  if (!status.ok()){
    delete db;
    NDN_THROW(std::runtime_error("DB cannot delete the block: " + name.toUri()));
  }
  delete db;
}

Interface
LedgerLevelDB::getInterface()
{
  Interface intf;
  intf.adder = std::bind(&LedgerLevelDB::addBlock, this, _1, _2);
  intf.getter = std::bind(&LedgerLevelDB::getBlock, this, _1);
  intf.deleter = std::bind(&LedgerLevelDB::deleteBlock, this, _1); 
  return intf;
}

} // namespace storage
} // namespace cledger
