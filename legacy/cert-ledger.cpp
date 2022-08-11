#include "cert-ledger/cert-ledger.hpp"
#include "util.hpp"

#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/logging.hpp>

NDN_LOG_INIT(cert_ledger.impl);

using namespace ndn;
namespace cert_ledger {

CertLedger::CertLedger(const Config &config,
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
    NDN_LOG_INFO("CertLedger Initialization Start");


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
        m_tailingRecords.emplace(genesisRecord.getRecordFullName());
    }
    NDN_LOG_INFO("STEP 2" << std::endl
                          << "- " << m_config.numGenesisBlock << " genesis records have been added to the CertLedger");
    NDN_LOG_INFO("CertLedger Initialization Succeed");
}

CertLedger::~CertLedger() = default;

ReturnCode CertLedger::createRecord(Record &record) {
    NDN_LOG_INFO("[CertLedger::createRecord] Add new record");

    std::vector<Name> tailingRecordList;
    for (const auto& i : m_tailingRecords) if (m_noPrevRecords.count(i) == 0) tailingRecordList.push_back(i);
    shuffleAndAddPrev(record, tailingRecordList);

    if (record.getPointersFromHeader().size() < m_config.precedingRecordNum) {
        std::vector<Name> lastRecordsList;
        for (const auto& i : m_lastNames) if (m_noPrevRecords.count(i) == 0) lastRecordsList.push_back(i);
        shuffleAndAddPrev(record, lastRecordsList);
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
    NDN_LOG_INFO("[CertLedger::createRecord] Added a new record:" << data->getFullName().toUri());

    // add new record into the ledger
    addRecordToDAG(record);

    //send sync interest
    m_dagSync.publishData(data->wireEncode(), data->getFreshnessPeriod(), m_config.peerPrefix, tlv::Data);
    return ReturnCode::noError(data->getFullName().toUri());
}

void CertLedger::shuffleAndAddPrev(Record &record, std::vector<Name>& recordList) {
    std::shuffle(recordList.begin(), recordList.end(), m_randomEngine);

    for (const auto &tailRecord : recordList) {
        record.addPointer(tailRecord);
        if (record.getPointersFromHeader().size() >= m_config.precedingRecordNum)
            break;
    }
}

optional<Record> CertLedger::getRecord(const Name &contentName) const {
    NDN_LOG_DEBUG("getRecord Called on " << contentName);
    auto re = m_backend.getRecord(contentName);
    if (re) {
        return Record(re);
    } else {
        return nullopt;
    }
}

bool CertLedger::hasRecord(const Name &contentName) const {
    auto dataPtr = m_backend.getRecord(Name(contentName));
    return dataPtr != nullptr;
}

std::list<Name> CertLedger::listRecord(const Name &prefix) const {
    return m_backend.listRecord(Name(prefix));
}

void CertLedger::onUpdate(const std::vector<ndn::svs::MissingDataInfo>& info) {
    for (const auto& stream : info) {
        for (svs::SeqNo i = stream.low; i <= stream.high; i++) {
            m_dagSync.fetchData(stream.nodeId, i, [&](const Data& syncData){
                if (syncData.getContentType() == tlv::Data) {
                    m_recordValidator->validate(Data(syncData.getContent().blockFromValue()), [this](const Data& data){
                        auto receivedData = std::make_shared<Data>(data);
                        try {
                            Record receivedRecord(receivedData);
                            receivedRecord.checkPointerCount(m_config.precedingRecordNum);

                            NDN_LOG_INFO("Received record " << receivedData->getFullName());
                            addRecordToDAG(receivedRecord);
                            verifyPreviousRecord(receivedRecord);

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

void CertLedger::addRecordToDAG(const Record& record) {
    NDN_LOG_INFO("Add record to Backend " << record.getRecordFullName());
    m_backend.putRecord(record.m_data);

    m_lastNames[m_lastNameTops] = record.getRecordFullName();
    m_lastNameTops = (m_lastNameTops + 1) % m_lastNames.size();

    m_tailingRecords.emplace(record.getRecordFullName());

    for (const auto& i : record.getPointersFromHeader()) {
        m_tailingRecords.erase(i);
    }
}

const Name &CertLedger::getPeerPrefix() const {
    return m_config.peerPrefix;
}

ndn::svs::SecurityOptions CertLedger::getSecurityOption(KeyChain& keychain, shared_ptr<ndn::security::Validator> recordValidator, Name peerPrefix) {
    ndn::svs::SecurityOptions option(keychain);
    option.validator = make_shared<::util::cxxValidator>(recordValidator);
    option.encapsulatedDataValidator = make_shared<::util::alwaysFailValidator>();
    option.dataSigner = std::make_shared<::util::KeyChainOptionSigner>(keychain, security::signingByIdentity(peerPrefix));
    option.interestSigner = option.dataSigner;
    option.pubSigner = std::make_shared<ndn::svs::BaseSigner>();
    return option;
}

void CertLedger::verifyPreviousRecord(const Record& record) {
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
        verifyPreviousRecord(Record(m_backend.getRecord(Record::getContentName(i))));
    }
}


}  // namespace cert-ledger