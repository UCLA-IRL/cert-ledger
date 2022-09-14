#include "sync/ledger-svs.hpp"

namespace cledger::sync {
NDN_LOG_INIT(cledger.sync);

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

LedgerSVSBase::LedgerSVSBase(const Name& syncPrefix,
                             const Name& nodePrefix,
                             ndn::Face& face,
                             const UpdateCallback& updateCallback,
                             const SecurityOptions& securityOptions,
                             std::shared_ptr<LedgerSVSDataStore> dataStore)
  : SVSyncBase(syncPrefix, Name(nodePrefix).append(syncPrefix), nodePrefix,
               face, updateCallback, securityOptions, std::move(dataStore))
{}

Name
LedgerSVSBase::getDataName(const NodeID& nid, const SeqNo& seqNo)
{
  return Name(nid).append(m_syncPrefix).appendNumber(seqNo);
}

Name
LedgerSVSBase::getMyDataName(const SeqNo& seqNo)
{
  return Name(m_dataPrefix).appendNumber(seqNo);
}

} // namespace cledger::sync