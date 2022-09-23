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
 * @author Tianyuan Yu
 */

#include "consumer.hpp"
#include "cledger-common.hpp"

namespace cledger::util::segment {
NDN_LOG_INIT(cledger.util);

Consumer::Consumer(Validator& validator, FinishCallback finishCb)
  : m_validator(validator)
  , m_finishCb(finishCb)
{
}

void
Consumer::run(const Name& versionedName, shared_ptr<PipelineInterests> pipeline)
{
  m_pipeline = std::move(pipeline);
  m_nextToPrint = 0;
  m_bufferedData.clear();

  m_pipeline->run(versionedName,
                  FORWARD_TO_MEM_FN(handleData),
                  [] (const std::string& msg) { NDN_THROW(std::runtime_error(msg)); });
}

void
Consumer::handleData(const Data& data)
{
  auto dataPtr = data.shared_from_this();
  NDN_LOG_TRACE("SegmentConsumer received " << data.getName());

  m_validator.validate(data,
    [this, dataPtr] (const Data& data) {
      if (data.getContentType() == ndn::tlv::ContentType_Nack) {
        NDN_THROW(std::runtime_error("Internal error of SegmentConsumer: " + data.getName().toUri()));
      }

      // 'data' passed to callback comes from DataValidationState and was not created with make_shared
      m_bufferedData[getSegmentFromPacket(data)] = dataPtr;
      writeInOrderData();
    },
    [] (const Data&, const ValidationError& error) {
      NDN_THROW(std::runtime_error("Internal error of SegmentConsumer: " + error.getInfo()));
    });
}

void
Consumer::writeInOrderData()
{
  for (auto it = m_bufferedData.begin();
       it != m_bufferedData.end() && it->first == m_nextToPrint;
       it = m_bufferedData.erase(it), ++m_nextToPrint) {
    NDN_LOG_TRACE("SegmentConsumer writes in order data " << it->second->getName());
    const Block& content = it->second->getContent();
    Buffer currSegmentBuffer(content.value(), content.value_size());
    m_outputBuffer.insert(m_outputBuffer.end(), currSegmentBuffer.begin(), currSegmentBuffer.end());
  }
  if (m_pipeline->allSegmentsReceived()) {
    NDN_LOG_DEBUG("SegmentConsumer receives all segments");
    auto block = make_shared<Block>(m_outputBuffer);
    m_finishCb(*block);
  }
}

} // namespace cledger::util::segment
