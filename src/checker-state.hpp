#ifndef CLEDGER_CHECKER_STATE_HPP
#define CLEDGER_CHECKER_STATE_HPP

#include "record.hpp"
#include "nack.hpp"
#include "error.hpp"
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-config.hpp>

namespace cledger::checker {

using onNackCallback = std::function<void(const Data&, const Nack&)>;
using onDataCallback = std::function<void(const Data&, const Data&)>;
using onFailureCallback = std::function<void(const Data&, const Error&)>;

class CheckerState : boost::noncopyable
{
public:
  const ssize_t CHECKER_MAX_RETRIES = 3;

  explicit
  CheckerState(ndn::Face& face,
               const Data& data,
               const onNackCallback onNack, 
               const onDataCallback onData, 
               const onFailureCallback onFailure);
  
  std::shared_ptr<Interest>
  makeInterest(const Name& ledgerPrefix);

  bool
  exhaustRetries()
  {
    return m_retryCount++ > CHECKER_MAX_RETRIES? true : false;
  }

  void
  onNack(const Nack& nack)
  {
    return m_nCb(m_data, nack);
  }

  void
  onData(const Data& data)
  {
    return m_dCb(m_data, data);
  }

  void
  onFailure(const Error& error)
  {
    return m_fCb(m_data, error);
  }

private:
  ndn::Face& m_face;

  Data m_data;
  onNackCallback m_nCb;
  onDataCallback m_dCb;
  onFailureCallback m_fCb;
  ssize_t m_retryCount = 0;
};

} // namespace cledger::checker

#endif // CLEDGER_CHECKER_STATE_HPP
