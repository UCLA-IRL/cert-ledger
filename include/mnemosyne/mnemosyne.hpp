#ifndef MNEMOSYNE_MNEMOSYNE_H_
#define MNEMOSYNE_MNEMOSYNE_H_

#include "mnemosyne/mnemosyne-dag-sync.hpp"
#include <ndn-svs/svspubsub.hpp>

using namespace ndn;
namespace mnemosyne {

class Mnemosyne {
  public:
    /**
     * Initialize a Mnemosyne instance from the config.
     * @p config, input, the configuration of multicast prefix, peer prefix, and settings of Mnemosyne behavior
     * @p keychain, input, the local NDN keychain instance
     * @p face, input, the localhost NDN face to send/receive NDN packets.
     * @p recordValidator, a validator that validates records from other nodes
     * @p eventValidator, a validator that validates events from clients
     */
    Mnemosyne(const Config &config, security::KeyChain &keychain, Face &network,
              std::shared_ptr<ndn::security::Validator> recordValidator,
              std::shared_ptr<ndn::security::Validator> eventValidator);

    virtual ~Mnemosyne();

  private:
    void onSubscriptionData(const svs::SVSPubSub::SubscriptionData& subData);

    ndn::svs::SecurityOptions getSecurityOption();

    void onRecordUpdate(Record record);

    bool seenEvent(const Name& name) const;

  protected:
    const Config m_config;
    security::KeyChain &m_keychain;
    MnemosyneDagSync m_dagSync;
    svs::SVSPubSub m_interfacePS;
    Scheduler m_scheduler;
    std::shared_ptr<ndn::security::Validator> m_eventValidator;

    //TODO convert to a database structure
    std::set<Name> m_eventSet;

    std::mt19937_64 m_randomEngine;
  };

} // namespace mnemosyne

#endif // MNEMOSYNE_MNEMOSYNE_H_