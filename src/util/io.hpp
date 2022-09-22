/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2022 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#ifndef CLEDGER_UTIL_IO_HPP
#define CLEDGER_UTIL_IO_HPP

#include <iostream>

#include "cledger-common.hpp"
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/io.hpp>
#include <ndn-cxx/security/transform/base64-encode.hpp>
#include <ndn-cxx/security/transform/buffer-source.hpp>
#include <ndn-cxx/security/transform/stream-sink.hpp>
namespace cledger::util {

template<typename T>
T
loadFromFile(const std::string& filename)
{
  try {
    if (filename == "-") {
      return ndn::io::loadTlv<T>(std::cin, ndn::io::BASE64);
    }

    std::ifstream file(filename);
    if (!file) {
      NDN_THROW(std::runtime_error("Cannot open '" + filename + "'"));
    }
    return ndn::io::loadTlv<T>(file, ndn::io::BASE64);
  }
  catch (const ndn::io::Error& e) {
    NDN_THROW_NESTED(std::runtime_error("Cannot load '" + filename +
                                        "': malformed TLV or not in base64 format (" + e.what() + ")"));
  }
}

ndn::security::Certificate
getCertificateFromPib(ssize_t& nStep,
                      const ndn::security::pib::Pib& pib, const Name& name,
                      bool isIdentityName, bool isKeyName, bool isCertName);

Name
captureKeyName(ssize_t& nStep, ndn::security::pib::Identity& identity);

Name
captureCertName(ssize_t& nStep, ndn::security::pib::Key& key);

} // namespace cledger::util
#endif // CLEDGER_UTIL_IO_HPP