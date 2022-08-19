#include "dag/dag-module.hpp"

namespace cledger::dag {
NDN_LOG_INIT(cledger.dag);

DagModule&
DagModule::add(Record& record)
{
  auto state = get(toStateName(record.getName()));
  state.record = record;
  switch (state.status) {
    case EdgeState::INITIALIZED:
      return onNewRecord(state);
    case EdgeState::LOADED:
      // duplicate, reject
      return *this;
    default:
      break;
  }
  return *this;
}


DagModule&
DagModule::setInterlockPolicy(const std::shared_ptr<InterlockPolicy> policy)
{
  m_policy = policy;
  return *this;
}

DagModule&
DagModule::setStorage(const std::shared_ptr<storage::LedgerStorage> storage)
{
  m_storage = storage;
  return *this;
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
    auto parent = get(toStateName(ptr));
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
      for (auto& ptr : get(f).record.getPointers()) {
        auto parent = get(toStateName(ptr));

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
    ret.push_front(get(a));
  }
  return ret;
}

std::list<Record>
DagModule::reap(const uint32_t threshold, bool removeFromWaitlist)
{
  auto consolidate = [this] (EdgeState& s) {
    try {
      m_storage->addBlock(s.stateName, encodeEdgeState(s));
    }
    catch (std::exception&) {
      NDN_LOG_DEBUG(s.stateName << " already exists");
      return;
    }
    NDN_LOG_DEBUG(s.stateName << " consolidated");
    m_buffer.erase(s.stateName);
  }; 

  std::list<Record> ret;
  for (auto& map : m_waitlist) {
    if (map.first >= threshold)  {
      for (auto& s : map.second) {
        auto state = get(s);
        ret.push_back(state.record);
        if (removeFromWaitlist) map.second.erase(s);
        if (m_storage) consolidate(state);
      }
    }
  }
  return ret;
}

EdgeState
DagModule::get(const Name& name)
{
  auto construct = [] (const Name& n) {
    EdgeState s;
    s.stateName = Name(n);
    s.status = EdgeState::INITIALIZED;
    return s;
  };

  if (m_buffer.find(name) != m_buffer.end()) {
    return m_buffer[name];
  }
  else if (m_storage) {
    try {
      auto block = m_storage->getBlock(name);
      return decodeEdgeState(block);
    }
    catch (std::exception& e) {
      return construct(name);
    }
  }
  else return construct(name);
}

void
DagModule::update(const Name& name, EdgeState state)
{
  m_buffer.insert_or_assign(name, state);
}

DagModule&
DagModule::onNewRecord(EdgeState& state)
{
  state.status = EdgeState::LOADED;
  update(state.stateName, state);
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
  NDN_LOG_TRACE("checking ancestors for " << state.stateName);
  auto ancestors = getAncestors(state);
  for (auto& a : ancestors) {
    NDN_LOG_TRACE("an ancestor " << a.stateName);
    a.descendants.insert(state.stateName);
    update(a.stateName, a);
    evaluateWaitlist(a);
  }
}

void
DagModule::evaluateWaitlist(EdgeState& state)
{
  for (auto& l : m_waitlist) {
    auto prev = l.second.find(state.stateName);
    if (prev != l.second.end()) {
      l.second.erase(prev);
    }
  }
  if (!m_policy) {
    NDN_THROW(std::runtime_error("Interlock policy has not been set"));
  }
  m_waitlist[m_policy->evaluate(state)].insert(state.stateName); 
}
} // namespace cledger::dag