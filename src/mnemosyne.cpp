#include "mnemosyne/mnemosyne.hpp"

#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/logging.hpp>
#include <utility>

NDN_LOG_INIT(mnemosyne.impl);

using namespace ndn;
namespace mnemosyne {


Mnemosyne::Mnemosyne(const Config &config, KeyChain &keychain, Face &network, std::shared_ptr<ndn::security::Validator> recordValidator, std::shared_ptr<ndn::security::Validator> eventValidator) :
        MnemosyneDagSync(config, keychain, network, std::move(recordValidator)),
        m_scheduler(network.getIoService()),
        m_interfacePS(config.interfacePrefix, config.peerPrefix, network, [](const auto& i){}),
        m_eventValidator(std::move(eventValidator))
{
    m_interfacePS.subscribeToPrefix(Name("/"), [&](const auto& d){ onSubscriptionData(d);});
}

void Mnemosyne::onSubscriptionData(const svs::SVSPubSub::SubscriptionData& subData) {
    m_eventValidator->validate(subData.data, [this](const Data& eventData){
        std::uniform_int_distribution<int> delayDistribution(0, 1000);
        NDN_LOG_INFO("Received event data " << eventData.getFullName());
        m_scheduler.schedule(time::milliseconds(delayDistribution(m_randomEngine)), [this, eventData]() {
            if (seenEvent(eventData.getFullName())) {
                NDN_LOG_INFO("Event data " << eventData.getFullName() << " found in DAG. ");
                return;
            } else {
                NDN_LOG_INFO("Event data " << eventData.getFullName() << " not found in DAG. Publishing...");
            }
            Record record(getPeerPrefix(), eventData);
            createRecord(record);
        });
    }, [](auto&& eventData, auto&& error){
        NDN_LOG_ERROR("Event data " << eventData.getFullName() << " verification error: " << error);
    });

}

Mnemosyne::~Mnemosyne() = default;

}  // namespace mnemosyne