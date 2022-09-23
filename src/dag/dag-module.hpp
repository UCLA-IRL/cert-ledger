#ifndef CLEDGER_DAG_EDGE_HPP
#define CLEDGER_DAG_EDGE_HPP

#include "record.hpp"
#include "dag/interlock-policy.hpp"
#include "storage/ledger-storage.hpp"
namespace cledger::dag {

class DagModule {
public:

  // TODO: need an explicit constructor
  DagModule(storage::Interface storageIntf, policy::Interface policyIntf);

  Name
  add(const Record& record);

  std::list<Record>
  harvestAbove(const uint32_t threshold, bool remove = false);

  std::list<Record>
  harvestBelow(const uint32_t threshold);

  std::set<Name>
  getWaitList(const uint32_t value)
  {
    try {
      return m_waitlist[value];
    }
    catch (const std::runtime_error& e) {
      return std::set<Name>();
    }
  }

  std::map<const uint32_t, std::set<Name>>
  getWaitList()
  {
    return m_waitlist;
  }

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::list<Name>
  getAncestors(EdgeState state);

  EdgeState
  getOrConstruct(const Name& name);

  void
  update(EdgeState state);

  DagModule&
  onNewRecord(EdgeState& state);

  void
  evaluateAncestors(EdgeState& state);

  void
  evaluateWaitlist(EdgeState& state);

  std::map<const uint32_t, std::set<Name>> m_waitlist;
  storage::Interface m_storageIntf;
  policy::Interface m_policyIntf;
};

std::ostream&
operator<<(std::ostream& os, const EdgeState& state);

} // namespace cledger::dag

#endif // CLEDGER_DAG_EDGE_HPP