#ifndef CLEDGER_COMMON_HPP
#define CLEDGER_COMMON_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/certificate.hpp>
#include <ndn-cxx/util/exception.hpp>
#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/optional.hpp>
#include <ndn-cxx/util/sha256.hpp>
#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-info.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>

#include "cledger-config.hpp"

#ifdef CLEDGER_HAVE_TESTS
#define CLEDGER_VIRTUAL_WITH_TESTS virtual
#define CLEDGER_PUBLIC_WITH_TESTS_ELSE_PROTECTED public
#define CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE public
#define CLEDGER_PROTECTED_WITH_TESTS_ELSE_PRIVATE protected
#else
#define CLEDGER_VIRTUAL_WITH_TESTS
#define CLEDGER_PUBLIC_WITH_TESTS_ELSE_PROTECTED protected
#define CLEDGER_PUBLIC_WITH_TESTS_ELSE_PRIVATE private
#define CLEDGER_PROTECTED_WITH_TESTS_ELSE_PRIVATE private
#endif

namespace cledger {

using ndn::Face;
using ndn::KeyChain;
using ndn::Block;
using ndn::Buffer;
using ndn::Data;
using ndn::Interest;
using ndn::Name;
using ndn::SignatureInfo;
using ndn::security::Certificate;
using ndn::util::Sha256;
using ndn::span;
using ndn::Scheduler;
// ndn-cxx moves to C++17 therefore ndn::optional and ndn::nullopt deprecated
using std::optional;
using std::nullopt;
using JsonSection = boost::property_tree::ptree;
using ndn::security::SigningInfo;
using ndn::security::Validator;
using ndn::security::ValidationError;

using ndn::make_span;

namespace time = ndn::time;
using namespace ndn::time_literals;
using namespace std::string_literals;

namespace tlv {

enum : uint32_t {
  GENERIC_RECORD = 1,
  GENESIS_RECORD = 4,
  REPLY_RECORD = 5,
};

enum : uint32_t {
  TLV_RECORD_TYPE = 211,
  TLV_RECORD_POINTER = 212,
  TLV_RECORD_PAYLOAD = 213
};

} // namespace tlv
} // namespace cledger

#endif // CLEDGER_COMMON_HPP
