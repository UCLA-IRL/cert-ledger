#ifndef CERT_LEDGER_CERT_LEDGER_H_
#define CERT_LEDGER_CERT_LEDGER_H_

#include "cert-ledger/cert-ledger-dag-sync.hpp"
#include <ndn-svs/svspubsub.hpp>

using namespace ndn;
namespace cert_ledger {

class Cert_ledger {
  public:
    /**
     * Initialize a Cert_ledger instance from the config.
     * @p config, input, the configuration of multicast prefix, peer prefix, and settings of Cert_ledger behavior
     * @p keychain, input, the local NDN keychain instance
     * @p face, input, the localhost NDN face to send/receive NDN packets.
     * @p recordValidator, a validator that validates records from other nodes
     * @p eventValidator, a validator that validates events from clients
     */
    Cert_ledger(const Config &config, security::KeyChain &keychain, Face &network,
              std::shared_ptr<ndn::security::Validator> recordValidator,
              std::shared_ptr<ndn::security::Validator> eventValidator);

    virtual ~Cert_ledger();

  private:
    void onSubscriptionData(const svs::SVSPubSub::SubscriptionData& subData);

    ndn::svs::SecurityOptions getSecurityOption();

    void onRecordUpdate(Record record);

    bool seenEvent(const Name& name) const;

  protected:
    const Config m_config;
    security::KeyChain &m_keychain;
    Cert_ledgerDagSync m_dagSync;
    svs::SVSPubSub m_interfacePS;
    Scheduler m_scheduler;
    std::shared_ptr<ndn::security::Validator> m_eventValidator;

    //TODO convert to a database structure
    std::set<Name> m_eventSet;

    std::mt19937_64 m_randomEngine;
  };

} // namespace cert-ledger

#endif // CERT_LEDGER_CERT_LEDGER_H_