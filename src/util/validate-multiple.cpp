#include "util/validate-multiple.hpp"

namespace cledger::util {
NDN_LOG_INIT(cledger.util);

void
validateMultipleData(ndn::security::Validator& validator,
                     const std::vector<Data>& dataVector, 
                     const ndn::security::DataValidationSuccessCallback& successCb,
                     const ndn::security::DataValidationFailureCallback& failureCb)
{
  uint32_t count = 0;
  uint32_t countTarget = dataVector.size();
  for (auto& item : dataVector) {
    validator.validate(item,
      [&count, countTarget, successCb] (const Data& data) {
        NDN_LOG_DEBUG("[validateMultipleData] Data " << data.getName() << " conforms to trust schema");
        if (++count < countTarget) {
          // not finished, continued
        }
        else {
          // finished
          successCb(data);
        }
      },
      failureCb);
  }
}
} // namespace ndnrevoke::util