#include "sync/sync-module.hpp"

namespace cledger::sync {
NDN_LOG_INIT(cledger.sync);

SyncModule::SyncModule(const SyncOptions &options, const SecurityOptions& secOps, ndn::Face& face,
                       storage::Interface storageIntf, const YieldRecordCallback& yield)
  : m_syncOptions(options)
  , m_secOptions(secOps)
  , m_face(face)
  , m_storageIntf(storageIntf)
  , m_yieldCb(yield)
{
  SVSPubSubOptions pubSubopts;
  pubSubopts.dataStore = std::make_shared<LedgerSVSDataStore>(m_storageIntf);
  m_ps = std::make_shared<SVSPubSub>(m_syncOptions.prefix, 
                                     m_syncOptions.id,
                                     m_face, [] (auto&&) {},
                                     pubSubopts, m_secOptions);
  // Subscribe to all data packets with prefix /chat (the "topic")
  m_ps->subscribe(Name("/"), [this] (const auto& subData) {
    // assume no segmentation
    auto name = subData.name;
    auto contentBlock = subData.data;
    m_yieldCb(Record(name, Block(contentBlock)));
  });
}

Name
SyncModule::publishRecord(Record& record)
{
  auto name = getNextName();
  auto content = record.prepareContent();
  content->encode();
  NDN_LOG_INFO("Publishing " << name);
  m_seqNo = m_ps->publish(name, make_span(reinterpret_cast<const uint8_t*>(content->data()), content->size()));
  return name;
}

Name
SyncModule::getNextName()
{
  return Name(m_syncOptions.id).appendSequenceNumber(m_seqNo + 1);
}

} // namespace cledger::sync