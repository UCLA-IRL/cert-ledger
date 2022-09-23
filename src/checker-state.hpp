#ifndef CLEDGER_CHECKER_STATE_HPP
#define CLEDGER_CHECKER_STATE_HPP

#include "record.hpp"
#include "nack.hpp"
#include "error.hpp"
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-config.hpp>

namespace cledger::checker {

using onSuccessCallback = std::function<void(const Data&, const Block&)>;
using onFailureCallback = std::function<void(const Data&, const Error&)>;

class CheckerState : boost::noncopyable
{
public:
  const ssize_t CHECKER_MAX_RETRIES = 3;

  explicit
  CheckerState(const Data& data,
               const onSuccessCallback onSuccess, 
               const onFailureCallback onFailure);
  
  std::shared_ptr<Interest>
  makeInterest(const Name& ledgerPrefix);

  bool
  exhaustRetries()
  {
    return m_retryCount++ > CHECKER_MAX_RETRIES? true : false;
  }

  void
  onSuccess(const Block& block)
  {
    return m_sCb(m_data, block);
  }

  void
  onFailure(const Error& error)
  {
    return m_fCb(m_data, error);
  }

private:
  Data m_data;
  onSuccessCallback m_sCb;
  onFailureCallback m_fCb;
  ssize_t m_retryCount = 0;
};

} // namespace cledger::checker

#endif // CLEDGER_CHECKER_STATE_HPP
