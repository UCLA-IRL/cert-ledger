#include "mnemosyne/mnemosyne.hpp"
#include "util.hpp"

#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/logging.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <utility>

NDN_LOG_INIT(mnemosyne.impl);

using namespace ndn;
namespace mnemosyne {


Mnemosyne::Mnemosyne(const Config &config, KeyChain &keychain, Face &network, std::shared_ptr<ndn::security::Validator> recordValidator, std::shared_ptr<ndn::security::Validator> eventValidator) :
        m_config(config),
        m_keychain(keychain),
        m_dagSync(m_config, keychain, network, std::move(recordValidator)),
        m_scheduler(network.getIoService()),
        m_interfacePS(config.interfacePrefix, config.peerPrefix, network, [](const auto& i){}, getSecurityOption()),
        m_eventValidator(std::move(eventValidator))
{
    m_interfacePS.subscribeToPrefix(Name("/"), [&](const auto& d){ onSubscriptionData(d);});
    m_dagSync.setOnRecordCallback([&](const auto& record) {onRecordUpdate(record);});
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
            Record record(m_dagSync.getPeerPrefix(), eventData);
            m_dagSync.createRecord(record);
        });
    }, [](auto&& eventData, auto&& error){
        NDN_LOG_ERROR("Event data " << eventData.getFullName() << " verification error: " << error);
    });

}

ndn::svs::SecurityOptions Mnemosyne::getSecurityOption() {
    ndn::svs::SecurityOptions option(m_keychain);
    option.validator = make_shared<::util::cxxValidator>(m_eventValidator);
    option.encapsulatedDataValidator = option.validator;
    option.dataSigner = std::make_shared<::util::KeyChainOptionSigner>(m_keychain, security::signingByIdentity(m_config.peerPrefix));
    option.interestSigner = option.dataSigner;
    option.pubSigner = option.dataSigner;
    return option;
}

void Mnemosyne::onRecordUpdate(Record record) {
    m_eventValidator->validate(Data(record.getContentItem()), [&](const auto& eventData){
        const auto& eventFullName = eventData.getFullName();
        m_eventSet.insert(eventFullName);
    }, [](const auto& data, const auto& error){
        NDN_LOG_INFO("Verification error on event record " << data.getFullName() << ": " << error);
    });
}

bool Mnemosyne::seenEvent(const Name& name) const{
    return m_eventSet.count(name);
}

Mnemosyne::~Mnemosyne() = default;

}  // namespace mnemosyne