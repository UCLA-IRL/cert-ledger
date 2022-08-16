#include "dag/edge.hpp"

namespace cledger::dag {
NDN_LOG_INIT(cledger.dag);

EdgeManager&
EdgeManager::add(Record& record)
{
  auto state = get(record.getName());
  state.pointers = record.getPointers();
  state.payload = record.getPayload();
  // is this a gensis record?
  for (auto& p : state.pointers) {
    if (p == state.name) {
      NDN_LOG_TRACE(state.name << " is genesis record");
      state.status = EdgeState::LOADED;
      update(state.name, state);
      m_waitlist[0].insert(state.name);
      return *this;
    }
  }

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

std::list<EdgeState>
EdgeManager::getAncestors(EdgeState state) 
{
  std::list<EdgeState> ret;
  std::vector<Name> ancestors;
  // every frontier must be expandable (ie. not a pending record)
  std::vector<Name> frontier;
  std::vector<Name> nextFrontier;
  uint32_t prevAncestors = 0;
  uint32_t currAncestors = 0;
  // the oldest being the first

  NDN_LOG_TRACE("ptr size " << state.pointers.size());
  // start
  for (auto& ptr : state.pointers) {
    ancestors.push_back(ptr);
    auto parent = get(ptr);
    // if this is pending, update its descendents
    if (parent.status == EdgeState::INITIALIZED) {
      NDN_LOG_TRACE(parent.name << " is a pending record, stop here...");
    }
    else {
      // we gonna expand this ptr in the next round.
      frontier.push_back(ptr);
    }
  }
  currAncestors = ancestors.size();
  // interation
  while (currAncestors > prevAncestors)
  {
    nextFrontier.clear();
    for (auto& f : frontier) {
      for (auto& ptr : get(f).pointers) {
        auto parent = get(ptr);

        // add to ancestor?
        bool hasSeen = false;
        for (auto& a : ancestors) {
          if (a == parent.name) {
            hasSeen = true;
            break;
          }
        }
        if (!hasSeen) {
          ancestors.push_back(parent.name);
        }

        // pending: update descendants
        // nonpending: add to next frontier
        if (parent.status == EdgeState::INITIALIZED) {
          NDN_LOG_TRACE(parent.name << " is a pending record, stop here...");
        }
        else {
          nextFrontier.push_back(parent.name);
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
EdgeManager::reap(const uint32_t threshold)
{
  std::list<Record> ret;
  for (auto& map : m_waitlist)
    if (map.first >= threshold) 
      for (auto& n : map.second) {
        auto item = get(n);
        Record r;
        r.setName(item.name);
        r.setPointers(item.pointers);
        r.setPayload(item.payload);
        ret.push_back(r);
      }
  return ret;
}

EdgeState
EdgeManager::get(const Name& name)
{
  if (m_buffer.find(name) != m_buffer.end())
    return m_buffer[name];
  // if cannot get it, then it must be pending or initialized 
  EdgeState s;
  s.name = name;
  s.status = EdgeState::INITIALIZED;
  return s;
}

void
EdgeManager::update(const Name& name, EdgeState state)
{
  m_buffer.insert_or_assign(name, state);
  // if (m_buffer.find(name) != m_buffer.end())
  //   m_buffer.insert_or_assign(name, state);
  // else {
  //   NDN_THROW(std::runtime_error("EdgeState for " + name.toUri() + " does not exists"));
  // }
}

EdgeManager&
EdgeManager::onNewRecord(EdgeState& state)
{
  state.status = EdgeState::LOADED;
  update(state.name, state);
  m_waitlist[0].insert(state.name);
  // check all ancestors
  NDN_LOG_TRACE("checking ancestors for " << state.name);
  auto ancestors = getAncestors(state);
  for (auto& a : ancestors) {
    NDN_LOG_TRACE("an ancestor " << a.name);
    // add pointers
    // beforeAdd = a.getDescendantProducers().size();
    a.descendants.insert(state.name);
    update(a.name, a);
    evaluateWaitlist(a);
  }
  return *this;
}

void
EdgeManager::evaluateWaitlist(EdgeState& state)
{
  for (auto& l : m_waitlist) {
    auto prev = l.second.find(state.name);
    if (prev != l.second.end()) {
      l.second.erase(prev);
    }
  }
  m_waitlist[state.descendants.size()].insert(state.name); 
}
// std::ostream&
// operator<<(std::ostream& os, const EdgeState& state)
// {
//   std::cerr << 
// }
} // namespace cledger::dag