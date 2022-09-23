/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
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
 */

#ifndef CLEDGER_UTIL_SEGMENT_SEGMENT_COMMON_HPP
#define CLEDGER_UTIL_SEGMENT_SEGMENT_COMMON_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cledger-common.hpp"

#include <boost/assert.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

#include <ndn-cxx/util/backports.hpp>
#include <ndn-cxx/util/exception.hpp>
#include <ndn-cxx/util/time.hpp>

namespace cledger::util::segment {

using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;
using std::to_string;

using std::size_t;

using boost::noncopyable;

} // namespace cledger::util::segment

#define FORWARD_TO_MEM_FN(func) \
  [this] (auto&&... args) { this->func(std::forward<decltype(args)>(args)...); }

#endif // CLEDGER_UTIL_SEGMENT_SEGMENT_COMMON_HPP
