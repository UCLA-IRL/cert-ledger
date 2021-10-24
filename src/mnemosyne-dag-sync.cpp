#include "mnemosyne/mnemosyne-dag-sync.hpp"

#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <utility>
#include <ndn-cxx/security/verification-helpers.hpp>
#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/logging.hpp>
#include <utility>
#include <algorithm>
#include <random>
#include <sstream>

NDN_LOG_INIT(mnemosyne.dagsync.impl);

using namespace ndn;
namespace mnemosyne {

MnemosyneDagSync::MnemosyneDagSync(const Config &config,
                     security::KeyChain &keychain,
                     Face &network)
        : m_config(config)
        , m_keychain(keychain)
        , m_backend(config.databasePath)
        , m_dagSync(config.syncPrefix, config.peerPrefix, network, [&](const auto& i){onUpdate(i);})
        , m_randomEngine(std::random_device()())
        , m_lastNameTops(0)
{
    NDN_LOG_INFO("Mnemosyne Initialization Start");

    if (config.numGenesisBlock < config.precedingRecordNum) {
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

    // randomly shuffle the tailing record list
    std::vector<Name> recordList = m_lastNames;
    std::shuffle(std::begin(recordList), std::end(recordList), m_randomEngine);

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
    addRecord(data);

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
        for (svs::SeqNo i = stream.low; i <= stream.high; i++)
        {
            m_dagSync.fetchData(stream.nodeId, i, [&](const Data& syncData){
                if (syncData.getContentType() == tlv::Data) {
                    auto receivedData = std::make_shared<Data>(syncData.getContent().blockFromValue());
                    // TODO validation of received Data
                    NDN_LOG_INFO("Received record " << receivedData->getFullName());
                    addRecord(receivedData);
                    Record record(receivedData);
                    auto eventFullName = Data(record.getContentItem()).getFullName();
                    m_eventSet.insert(eventFullName);
                }
            });
        }
    }
}

void MnemosyneDagSync::addRecord(const shared_ptr<Data>& recordData) {
    NDN_LOG_INFO("Add record " << recordData->getFullName());
    m_backend.putRecord(recordData);
    m_lastNames[m_lastNameTops] = recordData->getFullName();
    m_lastNameTops ++;
    m_lastNameTops = m_lastNameTops % m_lastNames.size();
}

const Name &MnemosyneDagSync::getPeerPrefix() const {
    return m_config.peerPrefix;
}

bool MnemosyneDagSync::seenEvent(const Name& name) const{
    return m_eventSet.count(name);
}

}  // namespace mnemosyne