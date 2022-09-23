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

#ifndef CLEDGER_UTIL_SEGMENT_CONSUMER_HPP
#define CLEDGER_UTIL_SEGMENT_CONSUMER_HPP

#include "pipeline-interests.hpp"

#include <ndn-cxx/security/validation-error.hpp>
#include <ndn-cxx/security/validator.hpp>

#include <map>

namespace cledger::util::segment {

/**
 * @brief Segmented version consumer
 *
 * Discover the latest version of the data published under a specified prefix, and retrieve all the
 * segments associated to that version. The segments are fetched in order and written to a
 * user-specified stream in the same order.
 */
class Consumer : noncopyable
{
public:
  using FinishCallback = std::function<void(Block& block)>;
  /**
   * @brief Create the consumer
   */
  explicit
  Consumer(Validator& validator, FinishCallback finishCb);

  /**
   * @brief Run the consumer
   */
  void
  run(const Name& versionedName, shared_ptr<PipelineInterests> pipeline);

private:
  void
  handleData(const Data& data);

  void
  writeInOrderData();

private:
  Validator& m_validator;
  Buffer m_outputBuffer;
  shared_ptr<PipelineInterests> m_pipeline;
  uint64_t m_nextToPrint = 0;

  std::map<uint64_t, shared_ptr<const Data>> m_bufferedData;
  FinishCallback m_finishCb;
};

} // namespace cledger::util::segment

#endif // CLEDGER_UTIL_SEGMENT_CONSUMER_HPP
