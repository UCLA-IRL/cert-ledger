#ifndef CLEDGER_UTIL_VALIDATE_MULTIPLE_HPP
#define CLEDGER_UTIL_VALIDATE_MULTIPLE_HPP

#include "cledger-common.hpp"
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
namespace cledger::util {
void
validateMultipleData(ndn::security::Validator& validator,
                     const std::vector<Data>& dataVector, 
                     const ndn::security::DataValidationSuccessCallback& successCb,
                     const ndn::security::DataValidationFailureCallback& failureCb);

} // namespace cledger::util
#endif // CLEDGER_UTIL_VALIDATE_MULTIPLE_HPP