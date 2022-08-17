#ifndef CLEDGER_TESTS_SVS_CORE_FIXTURE_HPP
#define CLEDGER_TESTS_SVS_CORE_FIXTURE_HPP

#include "cledger-common.hpp"
#include <ndn-svs/svsync-base.hpp>

namespace cledger::tests {

using ndn::svs::SVSyncBase;
using ndn::svs::NodeID;
using ndn::svs::SeqNo;
using ndn::svs::SecurityOptions;
using ndn::svs::VersionVector;
using ndn::svs::MissingDataInfo;
using ndn::svs::DataStore;

class SVSCoreFixture
{
public:

  explicit
  SVSCoreFixture(const Name& syncPrefix,
                    const SecurityOptions& securityOptions,
                    const NodeID& nid);

  ~SVSCoreFixture();

  SeqNo
  getSeqNo(const NodeID& nid) const;

  void
  updateSeqNo(const SeqNo& seq, const NodeID& nid);

  std::shared_ptr<Interest>
  makeSyncInterest();

protected:

  // Communication
  const Name m_syncPrefix;
  const SecurityOptions m_securityOptions;
  const NodeID m_id;

  // State
  VersionVector m_vv;
};

} // namespace cledger::tests

#endif // CLEDGER_TESTS_SVS_CORE_FIXTURE_HPP
