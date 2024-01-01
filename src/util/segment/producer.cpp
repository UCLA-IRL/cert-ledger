/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016-2022, Regents of the University of California,
 *                          Colorado State University,
 *                          University Pierre & Marie Curie, Sorbonne University.
 *
 * This file is part of ndn-tools (Named Data Networking Essential Tools).
 * See AUTHORS.md for complete list of ndn-tools authors and contributors.
 *
 * ndn-tools is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndn-tools is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndn-tools, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Wentao Shang
 * @author Steve DiBenedetto
 * @author Andrea Tosatto
 * @author Davide Pesavento
 * @author Klaus Schneider
 * @author Chavoosh Ghasemi
 * @author Tianyuan Yu
 */

#include "producer.hpp"
#include "cledger-common.hpp"
#include <ndn-cxx/metadata-object.hpp>

namespace cledger::util::segment {

NDN_LOG_INIT(cledger.util);

Producer::Producer(const Name& prefix, Face& face, KeyChain& keyChain, const Block& block,
                   const Options& opts)
  : m_face(face)
  , m_keyChain(keyChain)
  , m_options(opts)
{
  if (!prefix.empty() && prefix[-1].isVersion()) {
    m_prefix = prefix.getPrefix(-1);
    m_versionedPrefix = prefix;
  }
  else {
    m_prefix = prefix;
    m_versionedPrefix = Name(m_prefix).appendVersion();
  }

  populateStore(block);

  // register m_prefix without interest handler
  auto prefixId = m_face.registerPrefix(m_prefix, [this] (auto&&...) {
    auto filterId = m_face.setInterestFilter(m_prefix, [this] (const auto&, const auto& interest) {
      processSegmentInterest(interest);
    });
    m_interestFilterHandles.push_back(filterId);
  },
  [] (const Name& prefix, const auto& reason) {
    NDN_LOG_ERROR("SegmentProducer failed to register prefix '" << prefix << "' (" << reason << ")");
  });
  m_registeredPrefixHandles.push_back(prefixId);

  // // match Interests whose name is exactly m_prefix
  // auto filterId = m_face.setInterestFilter(ndn::InterestFilter(m_prefix, ""), [this] (const auto&, const auto& interest) {
  //   processSegmentInterest(interest);
  // });
  // m_interestFilterHandles.push_back(filterId);

  NDN_LOG_TRACE("SegmentProducer Data published with name: " << m_versionedPrefix);
}

Producer::~Producer()
{
  NDN_LOG_TRACE("SegmentProducer deconstructing, cancel all filters and prefixes...");
  for (auto& handle : m_registeredPrefixHandles) {
    handle.cancel();
  }
  for (auto& handle : m_interestFilterHandles) {
    handle.cancel();
  }
}

void
Producer::processSegmentInterest(const Interest& interest)
{
  BOOST_ASSERT(!m_store.empty());
  NDN_LOG_TRACE("SegmentProducer Interest: " << interest);

  const Name& name = interest.getName();
  shared_ptr<Data> data;

  if (name.size() == m_versionedPrefix.size() + 1 && name[-1].isSegment()) {
    const auto segmentNo = static_cast<size_t>(interest.getName()[-1].toSegment());
    // specific segment retrieval
    if (segmentNo < m_store.size()) {
      data = m_store[segmentNo];
    }
  }
  else if (interest.matchesData(*m_store[0])) {
    // unspecified version or segment number, return first segment
    data = m_store[0];
  }

  if (data != nullptr) {
    NDN_LOG_TRACE("SegmentProducer Data: " << data->getName());
    m_face.put(*data);
  }
  else {
    m_face.put(ndn::lp::Nack(interest));
  }
}

void
Producer::populateStore(const Block& block)
{
  BOOST_ASSERT(m_store.empty());
  NDN_LOG_TRACE("SegmentProducer loading input..");

  std::vector<uint8_t> buffer(m_options.maxSegmentSize);
  
  int total = (block.size() - 1) / m_options.maxSegmentSize + 1;
  int count = 0;
  while (count < total) {
    int copyOffset = count++ * m_options.maxSegmentSize;
    int copySize = m_options.maxSegmentSize < block.size() - copyOffset?
                   m_options.maxSegmentSize : block.size() - copyOffset;
    NDN_LOG_DEBUG("SegmentProducer created No." << count << " chunk with " << copySize << " Bytes");
    std::memcpy(buffer.data(), block.data() + copyOffset, copySize);
    auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(m_store.size()));
    data->setFreshnessPeriod(m_options.freshnessPeriod);
    data->setContent(make_span(buffer.data(), copySize));
    m_store.push_back(data);
    buffer.clear();
  }

  if (m_store.empty()) {
    auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(0));
    data->setFreshnessPeriod(m_options.freshnessPeriod);
    m_store.push_back(data);
  }

  auto finalBlockId = Name::Component::fromSegment(m_store.size() - 1);
  for (const auto& data : m_store) {
    data->setFinalBlock(finalBlockId);
    m_keyChain.sign(*data, m_options.signingInfo);
  }
  NDN_LOG_DEBUG("SegmentProducer created " << m_store.size() << " chunks for prefix " << m_prefix);
}

} // namespace ndn::chunks
