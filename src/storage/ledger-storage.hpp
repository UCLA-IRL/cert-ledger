#ifndef CLEDGER_STORAGE_HPP
#define CLEDGER_STORAGE_HPP

#include "cledger-common.hpp"

namespace cledger {
namespace storage {

using Adder = std::function<void(const Name&, const Block&)>;
using Getter = std::function<Block(const Name&)>;
using Deleter = std::function<void(const Name&)>;
struct Interface {
  Adder adder;
  Getter getter;
  Deleter deleter;
};

class LedgerStorage : boost::noncopyable
{
public: 

  virtual void
  addBlock(const Name& name, const Block& block) = 0;

  virtual Block
  getBlock(const Name& name) = 0;

  virtual void
  deleteBlock(const Name& name) = 0;

  virtual Interface
  getInterface() = 0;

public: // factory
  template<class LedgerStorageType>
  static void
  registerLedgerStorage(const std::string& ledgerStorageType = LedgerStorageType::STORAGE_TYPE)
  {
    LedgerStorageFactory& factory = getFactory();
    factory[ledgerStorageType] = [] (const Name& ledgerName, const std::string& path) {
      return std::make_unique<LedgerStorageType>(ledgerName, path);
    };
  }

  static std::unique_ptr<LedgerStorage>
  createLedgerStorage(const std::string& ledgerStorageType, const Name& ledgerName, const std::string& path);

  virtual
  ~LedgerStorage() = default;

private:
  using LedgerStorageCreateFunc = std::function<std::unique_ptr<LedgerStorage> (const Name&, const std::string&)>;
  using LedgerStorageFactory = std::map<std::string, LedgerStorageCreateFunc>;

  static LedgerStorageFactory&
  getFactory();
};

#define CLEDGER_REGISTER_STORAGE(C)                         \
static class Cledger ## C ## LedgerStorageRegistrationClass        \
{                                                                \
public:                                                          \
  Cledger ## C ## LedgerStorageRegistrationClass()                 \
  {                                                              \
    ::cledger::storage::LedgerStorage::registerLedgerStorage<C>();          \
  }                                                              \
} g_Cledger ## C ## LedgerStorageRegistrationVariable

} // namespace storage
} // namespace cledger

#endif // CLEDGER_LEDGER_STORAGE_HPP
