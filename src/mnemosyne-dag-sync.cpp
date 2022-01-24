#include "mnemosyne/mnemosyne-dag-sync.hpp"
#include "util.hpp"

#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <utility>
#include <ndn-cxx/security/verification-helpers.hpp>
#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/logging.hpp>
#include <algorithm>
#include <random>
#include <sstream>

NDN_LOG_INIT(mnemosyne.dagsync.impl);

using namespace ndn;
namespace mnemosyne {

MnemosyneDagSync::MnemosyneDagSync(const Config &config,
                     security::KeyChain &keychain,
                     Face &network,
                     std::shared_ptr<ndn::security::Validator> recordValidator)
        : m_config(config)
        , m_keychain(keychain)
        , m_backend(config.databasePath)
        , m_recordValidator(recordValidator)
        , m_dagSync(config.syncPrefix, config.peerPrefix, network, [&](const auto& i){onUpdate(i);}, getSecurityOption(keychain, recordValidator, config.peerPrefix))
        , m_randomEngine(std::random_device()())
        , m_lastNameTops(0)
{
    NDN_LOG_INFO("Mnemosyne Initialization Start");


    if (config.numGenesisBlock < config.precedingRecordNum || config.precedingRecordNum <= 1) {
        NDN_THROW(std::runtime_error("Bad config"));
    }

    //****STEP 2****
    // Make the genesis data
    for (int i = 0; i < m_config.numGenesisBlock; i++) {
        GenesisRecord genesisRecord(i);
        auto data = make_shared<Data>(genesisRecord.getRecordName());
        auto contentBlock = makeEmptyBlock(tlv::Content);
        genesisRecord.wireEncode(contentBlock);
        data->setContent(contentBlock);
        m_keychain.sign(*data, signingWithSha256());
        genesisRecord.m_data = data;
        m_backend.putRecord(data);
        m_lastNames.push_back(genesisRecord.getRecordFullName());
    }
    NDN_LOG_INFO("STEP 2" << std::endl
                          << "- " << m_config.numGenesisBlock << " genesis records have been added to the Mnemosyne");
    NDN_LOG_INFO("Mnemosyne Initialization Succeed");
}

MnemosyneDagSync::~MnemosyneDagSync() = default;

ReturnCode MnemosyneDagSync::createRecord(Record &record) {
    NDN_LOG_INFO("[MnemosyneDagSync::createRecord] Add new record");

    if (!m_selfLastName.empty()) record.addPointer(m_selfLastName);

    // randomly shuffle the tailing record list
    std::vector<Name> recordList;
    for (const auto& i : m_lastNames) if (m_noPrevRecords.count(i) == 0) recordList.push_back(i);
    std::shuffle(recordList.begin(), recordList.end(), m_randomEngine);

    for (const auto &tailRecord : recordList) {
        record.addPointer(tailRecord);
        if (record.getPointersFromHeader().size() >= m_config.precedingRecordNum)
            break;
    }

    auto data = make_shared<Data>(record.getRecordName());
    auto contentBlock = makeEmptyBlock(tlv::Content);
    record.wireEncode(contentBlock);
    data->setContent(contentBlock);
    data->setFreshnessPeriod(time::minutes(5));

    // sign the packet with peer's key
    try {
        m_keychain.sign(*data, security::signingByIdentity(m_config.peerPrefix));
    }
    catch (const std::exception &e) {
        return ReturnCode::signingError(e.what());
    }
    record.m_data = data;
    NDN_LOG_INFO("[MnemosyneDagSync::createRecord] Added a new record:" << data->getFullName().toUri());

    // add new record into the ledger
    addSelfRecord(data);

    //send sync interest
    m_dagSync.publishData(data->wireEncode(), data->getFreshnessPeriod(), m_config.peerPrefix, tlv::Data);
    return ReturnCode::noError(data->getFullName().toUri());
}

optional<Record> MnemosyneDagSync::getRecord(const std::string &recordName) const {
    NDN_LOG_DEBUG("getRecord Called on " << recordName);
    return m_backend.getRecord(recordName);
}

bool MnemosyneDagSync::hasRecord(const std::string &recordName) const {
    auto dataPtr = m_backend.getRecord(Name(recordName));
    return dataPtr != nullptr;
}

std::list<Name> MnemosyneDagSync::listRecord(const std::string &prefix) const {
    return m_backend.listRecord(Name(prefix));
}

void MnemosyneDagSync::onUpdate(const std::vector<ndn::svs::MissingDataInfo>& info) {
    for (const auto& stream : info) {
        for (svs::SeqNo i = stream.low; i <= stream.high; i++) {
            m_dagSync.fetchData(stream.nodeId, i, [&](const Data& syncData){
                if (syncData.getContentType() == tlv::Data) {
                    m_recordValidator->validate(Data(syncData.getContent().blockFromValue()), [this](const Data& data){
                        auto receivedData = std::make_shared<Data>(data);
                        try {
                            Record receivedRecord(receivedData);
                            receivedRecord.checkPointerCount(m_config.precedingRecordNum);
                            verifyPreviousRecord(receivedRecord);

                            NDN_LOG_INFO("Received record " << receivedData->getFullName());
                            addReceivedRecord(receivedData);

                            if (m_onRecordCallback) {
                                m_onRecordCallback(receivedRecord);
                            }
                        } catch (const std::exception& e) {
                            NDN_LOG_ERROR("bad record received" << receivedData->getFullName() << ": " << e.what());
                        }
                    }, [](const Data& data, const ndn::security::ValidationError& error){
                        NDN_LOG_ERROR("Verification error on Received record " << data.getFullName() << ": " << error.getInfo());
                    });
                }
            });
        }
    }
}

void MnemosyneDagSync::addSelfRecord(const shared_ptr<Data> &data) {
    NDN_LOG_INFO("Add self record " << data->getFullName());
    m_backend.putRecord(data);
    m_selfLastName = data->getFullName();
}

void MnemosyneDagSync::addReceivedRecord(const shared_ptr<Data>& recordData) {
    NDN_LOG_INFO("Add received record " << recordData->getFullName());
    m_backend.putRecord(recordData);
    m_lastNames[m_lastNameTops] = recordData->getFullName();
    m_lastNameTops = (m_lastNameTops + 1) % m_lastNames.size();
}

const Name &MnemosyneDagSync::getPeerPrefix() const {
    return m_config.peerPrefix;
}

ndn::svs::SecurityOptions MnemosyneDagSync::getSecurityOption(KeyChain& keychain, shared_ptr<ndn::security::Validator> recordValidator, Name peerPrefix) {
    ndn::svs::SecurityOptions option(keychain);
    option.validator = make_shared<::util::cxxValidator>(recordValidator);
    option.encapsulatedDataValidator = make_shared<::util::alwaysFailValidator>();
    option.dataSigner = std::make_shared<::util::KeyChainOptionSigner>(keychain, security::signingByIdentity(peerPrefix));
    option.interestSigner = option.dataSigner;
    option.pubSigner = std::make_shared<ndn::svs::BaseSigner>();
    return option;
}

void MnemosyneDagSync::verifyPreviousRecord(const Record& record) {
    for (const auto& i : record.getPointersFromHeader()) {
        if (m_noPrevRecords.count(i) || !m_backend.getRecord(i)) {
            m_waitingReferencedRecords.emplace(i, record.getRecordFullName());
            m_noPrevRecords.emplace(record.getRecordFullName());
            return;
        }
    }

    std::list<Name> waitingList;
    if (m_waitingReferencedRecords.count(record.getRecordFullName()) > 0) {
        for (auto it = m_waitingReferencedRecords.find(record.getRecordFullName()); it->first == record.getRecordFullName(); m_waitingReferencedRecords.erase(it ++)) {
            waitingList.push_back(it->second);
            m_noPrevRecords.erase(it->second);

        }
    }

    for (const auto& i : waitingList) {
        verifyPreviousRecord(m_backend.getRecord(i));
    }
}


}  // namespace mnemosyne