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
 * @author Andrea Tosatto
 * @author Davide Pesavento
 * @author Tianyuan Yu
 */

#ifndef CLEDGER_UTIL_SEGMENT_OPTIONS_HPP
#define CLEDGER_UTIL_SEGMENT_OPTIONS_HPP

#include "util/segment/segment-common.hpp"

namespace cledger::util::segment {

struct Options : noncopyable
{
  // Common options
  time::milliseconds interestLifetime = ndn::DEFAULT_INTEREST_LIFETIME;
  int maxRetriesOnTimeoutOrNack = 15;
  bool mustBeFresh = false;

  // Fixed pipeline options
  size_t maxPipelineSize = 1;
};

} // namespace cledger::util::segment

#endif // CLEDGER_UTIL_SEGMENT_OPTIONS_HPP
