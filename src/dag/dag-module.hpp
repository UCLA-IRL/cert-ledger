#ifndef CLEDGER_DAG_EDGE_HPP
#define CLEDGER_DAG_EDGE_HPP

#include "record.hpp"
#include "dag/interlock-policy.hpp"
#include "storage/ledger-storage.hpp"
namespace cledger::dag {

class DagModule {
public:
  DagModule&
  add(Record& record);

  DagModule&
  setInterlockPolicy(const std::shared_ptr<InterlockPolicy> policy); 

  DagModule&
  setStorage(const std::shared_ptr<storage::LedgerStorage> policy); 

  std::list<Record>
  reap(const uint32_t threshold, bool removeFromWaitlist = false);

CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::list<EdgeState>
  getAncestors(EdgeState state);

  EdgeState
  get(const Name& name);

  void
  update(const Name& name, EdgeState state);


  DagModule&
  onNewRecord(EdgeState& state);

  void
  evaluateAncestors(EdgeState& state);

  void
  evaluateWaitlist(EdgeState& state);

  std::map<Name, EdgeState> m_buffer;
  std::map<const uint32_t, std::set<Name>> m_waitlist;
  std::shared_ptr<InterlockPolicy> m_policy;
  std::shared_ptr<storage::LedgerStorage> m_storage;
};

std::ostream&
operator<<(std::ostream& os, const EdgeState& state);

} // namespace cledger::dag

#endif // CLEDGER_DAG_EDGE_HPP