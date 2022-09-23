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
 *//* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "util/io.hpp"

namespace cledger::util {

ndn::security::Certificate
getCertificateFromPib(ssize_t& nStep,
                      const ndn::security::pib::Pib& pib, const Name& name,
                      bool isIdentityName, bool isKeyName, bool isCertName)
{
  if (isIdentityName) {
    auto identity = pib.getIdentity(name);
    if (identity.getKeys().size() > 1) {
      return getCertificateFromPib(nStep, pib,
          captureKeyName(nStep, identity), false, true, false);
    }
    else {
      return getCertificateFromPib(nStep, pib,
        identity.getDefaultKey().getName(), false, true, false);    
    }
  }
  else if (isKeyName) {
    auto key = pib.getIdentity(ndn::security::extractIdentityFromKeyName(name))
                  .getKey(name);
		if (key.getCertificates().size() > 1) {
			return getCertificateFromPib(nStep, pib,
        captureCertName(nStep, key), false, false, true);
		}
    else {
      return getCertificateFromPib(nStep, pib,
        key.getDefaultCertificate().getName(), false, false, true);
    }
  }
  else if (isCertName) {
    return pib.getIdentity(ndn::security::extractIdentityFromCertName(name))
           .getKey(ndn::security::extractKeyNameFromCertName(name))
           .getCertificate(name);
  }
  // should never be called
  return pib.getIdentity(ndn::security::extractIdentityFromCertName(name))
           .getKey(ndn::security::extractKeyNameFromCertName(name))
           .getCertificate(name);
}

Name
captureKeyName(ssize_t& nStep, ndn::security::pib::Identity& identity)
{
  size_t count = 0;
  std::cerr << "***************************************\n"
            << "Step " << nStep++ << ": KEY SELECTION" << std::endl;
  for (const auto& key : identity.getKeys()) {
    std::cerr << "> Index: " << count++ << std::endl
              << ">> Key Name:";
    if (key == identity.getDefaultKey()) {
      std::cerr << "  +->* ";
    }
    else {
      std::cerr << "  +->  ";
    }
    std::cerr << key.getName() << std::endl;
  }

  std::cerr << "Please type in the key's index that you want to select:\n";
  std::string indexStr = "";
  std::string indexStrLower = "";
  size_t keyIndex;
  getline(std::cin, indexStr);

  indexStrLower = indexStr;
  boost::algorithm::to_lower(indexStrLower);
	try {
		keyIndex = std::stoul(indexStr);
	}
	catch (const std::exception&) {
		std::cerr << "Your input is not valid index. Exit" << std::endl;
		exit(1);
	}

	if (keyIndex >= count) {
		std::cerr << "Your input is not an existing index. Exit" << std::endl;
		exit(1);
	}
	else {
		auto itemIterator = identity.getKeys().begin();
		std::advance(itemIterator, keyIndex);
		auto targetKeyItem = *itemIterator;
		return targetKeyItem.getName();
	}
}

Name
captureCertName(ssize_t& nStep, ndn::security::pib::Key& key)
{
  size_t count = 0;
  std::cerr << "***************************************\n"
            << "Step " << nStep++ << ": CERTIFICATE SELECTION" << std::endl;
  for (const auto& cert : key.getCertificates()) {
    std::cerr << "> Index: " << count++ << std::endl
              << ">> Certificate Name:";
    if (cert == key.getDefaultCertificate()) {
      std::cerr << "  +->* ";
    }
    else {
      std::cerr << "  +->  ";
    }
    std::cerr << cert.getName() << std::endl;
  }

  std::cerr << "Please type in the key's index that you want to select:\n";
  std::string indexStr = "";
  std::string indexStrLower = "";
  size_t certIndex;
  getline(std::cin, indexStr);

  indexStrLower = indexStr;
  boost::algorithm::to_lower(indexStrLower);
	try {
		certIndex = std::stoul(indexStr);
	}
	catch (const std::exception&) {
		std::cerr << "Your input is not valid index. Exit" << std::endl;
		exit(1);
	}

	if (certIndex >= count) {
		std::cerr << "Your input is not an existing index. Exit" << std::endl;
		exit(1);
	}
	else {
		auto itemIterator = key.getCertificates().begin();
		std::advance(itemIterator, certIndex);
		auto targetCertItem = *itemIterator;
		return targetCertItem.getName();
	}
}
} // namespace cledger::util