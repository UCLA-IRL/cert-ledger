#include "sync/ledger-svs.hpp"

namespace cledger::sync {

LedgerSVSDataStore::LedgerSVSDataStore(storage::Interface storageIntf)
  : m_storageIntf(storageIntf)
{}

std::shared_ptr<const Data>
LedgerSVSDataStore::find(const Interest& interest)
{
  try {
    auto data = std::make_shared<const Data>(m_storageIntf.getter(interest.getName()));
    return data;
  }
  catch (const std::runtime_error& e) {
    return nullptr;
  }
}

void
LedgerSVSDataStore::insert(const Data& data)
{
  m_storageIntf.adder(data.getName(), data.wireEncode());
}

} // namespace cledger::sync