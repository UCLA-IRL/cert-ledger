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

  DagModule&
  add(const Record& record);

  std::list<Record>
  reap(const uint32_t threshold, bool removeFromWaitlist = false);

  std::set<Name>
  getWaitList(const uint32_t value)
  {
    return m_waitlist[value];
  }

  std::map<const uint32_t, std::set<Name>>
  getWaitList()
  {
    return m_waitlist;
  }

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::list<EdgeState>
  getAncestors(EdgeState state);

  EdgeState
  getOrConstruct(const Name& name);

  void
  update(const Name& name, EdgeState state);


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