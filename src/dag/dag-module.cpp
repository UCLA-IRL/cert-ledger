#include "dag/dag-module.hpp"
#include "dag/edge-state-list.hpp"

namespace cledger::dag {
NDN_LOG_INIT(cledger.dag);

DagModule::DagModule(storage::Interface storageIntf, policy::Interface policyIntf)
 : m_storageIntf(storageIntf)
 , m_policyIntf(policyIntf)
{
}

Name
DagModule::add(const Record& record)
{
  auto state = getOrConstruct(toStateName(record.getName()));
  state.record = record;
  switch (state.status) {
    case EdgeState::INITIALIZED:
      onNewRecord(state);
      return state.stateName;
    case EdgeState::LOADED:
      // duplicate, reject
      return state.stateName;
    default:
      break;
  }
  return state.stateName;
}

std::list<EdgeState>
DagModule::getAncestors(EdgeState state) 
{
  std::list<EdgeState> ret;
  std::vector<Name> ancestors;
  // every frontier must be expandable (ie. not a pending record)
  std::vector<Name> frontier;
  std::vector<Name> nextFrontier;
  uint32_t prevAncestors = 0;
  uint32_t currAncestors = 0;
  // the oldest being the first

  // start
  for (auto& ptr : state.record.getPointers()) {
    ancestors.push_back(toStateName(ptr));
    auto parent = getOrConstruct(toStateName(ptr));
    // if this is pending, update its descendents
    if (parent.status == EdgeState::INITIALIZED) {
      NDN_LOG_TRACE(parent.stateName << " is a pending state, stop here...");
    }
    else {
      // we gonna expand this ptr in the next round.
      frontier.push_back(toStateName(ptr));
    }
  }
  currAncestors = ancestors.size();
  // interation
  while (currAncestors > prevAncestors)
  {
    nextFrontier.clear();
    for (auto& f : frontier) {
      for (auto& ptr : getOrConstruct(f).record.getPointers()) {
        auto parent = getOrConstruct(toStateName(ptr));

        // add to ancestor?
        bool hasSeen = false;
        for (auto& a : ancestors) {
          if (a == parent.stateName) {
            hasSeen = true;
            break;
          }
        }
        if (!hasSeen) {
          ancestors.push_back(parent.stateName);
        }

        if (parent.status == EdgeState::INITIALIZED) {
          NDN_LOG_TRACE(parent.stateName << " is a pending state, stop here...");
        }
        else {
          nextFrontier.push_back(parent.stateName);
        }
      }
    }
    prevAncestors = currAncestors;
    currAncestors = ancestors.size();
    frontier = nextFrontier;
  }

  for (auto& a : ancestors) {
    ret.push_front(getOrConstruct(a));
  }
  return ret;
}

std::list<Record>
DagModule::harvest(const uint32_t threshold, bool remove)
{
  std::list<Record> ret;
  for (auto& map : m_waitlist) {
    std::list<Name> rm;
    if (map.first >= threshold)  {
      for (auto& s : map.second) {
        auto state = getOrConstruct(s);
        ret.push_back(state.record);
        if (remove) rm.push_back(s);
      }
      for (auto& n : rm) map.second.erase(n);
    }
  }
  return ret;
}

EdgeState
DagModule::getOrConstruct(const Name& name)
{
  auto construct = [] (const Name& n) {
    EdgeState s;
    s.stateName = Name(n);
    s.status = EdgeState::INITIALIZED;
    return s;
  };
  try {
    auto block = m_storageIntf.getter(name);
    return decodeEdgeState(block);
  }
  catch (std::exception& e) {
    auto s = construct(name);
    m_storageIntf.adder(s.stateName, encodeEdgeState(s));
    return s;
  }
}

void
DagModule::update(EdgeState state)
{
  m_storageIntf.deleter(state.stateName);
  m_storageIntf.adder(state.stateName, encodeEdgeState(state));
}

DagModule&
DagModule::onNewRecord(EdgeState& state)
{
  NDN_LOG_TRACE("Processing EdgeState " << state.stateName);
  state.status = EdgeState::LOADED;
  update(state);
  evaluateWaitlist(state);
  // if this is a genesis record
  if (!state.record.isGenesis()) {
    evaluateAncestors(state);
  }
  return *this;
}

void
DagModule::evaluateAncestors(EdgeState& state)
{
  NDN_LOG_TRACE("Checking ancestors for " << state.stateName);
  auto ancestors = getAncestors(state);
  for (auto& a : ancestors) {
    NDN_LOG_TRACE("Adding a descendant " << state.stateName << " for " << a.stateName
                  << ", current descendant size is " << a.descendants.size());
    a.descendants.insert(state.stateName);
    update(a);
    evaluateWaitlist(a);
  }
}

void
DagModule::evaluateWaitlist(EdgeState& state)
{
  NDN_LOG_TRACE("Evaluating waitlist for " << state.stateName);
  for (auto& l : m_waitlist) {
    auto prev = l.second.find(state.stateName);
    if (prev != l.second.end()) {
      l.second.erase(state.stateName);
    }
  }
  m_waitlist[m_policyIntf.evaluater(state)].insert(state.stateName);
}
} // namespace cledger::dag