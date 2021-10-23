#include "mnemosyne/mnemosyne.hpp"

#include <algorithm>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <utility>
#include <ndn-cxx/security/verification-helpers.hpp>
#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/logging.hpp>
#include <random>
#include <sstream>

NDN_LOG_INIT(mnemosyne.impl);

using namespace ndn;
namespace mnemosyne {

Mnemosyne::Mnemosyne(const Config &config,
                     security::KeyChain &keychain,
                     Face &network)
        : m_config(config), m_keychain(keychain), m_network(network), m_scheduler(network.getIoService()),
          m_backend(config.databasePath),
          m_sync(config.syncPrefix, config.peerPrefix, network, [&](const auto& i){onUpdate(i);}) {
    NDN_LOG_INFO("Mnemosyne Initialization Start");

    if (config.numGenesisBlock < config.precedingRecordNum) {
        NDN_THROW(std::runtime_error("Bad config"));
    }

    //****STEP 2****
    // Make the genesis data
    for (int i = 0; i < m_config.numGenesisBlock; i++) {
        GenesisRecord genesisRecord(i);
        auto data = make_shared<Data>(genesisRecord.m_recordName);
        auto contentBlock = makeEmptyBlock(tlv::Content);
        genesisRecord.wireEncode(contentBlock);
        data->setContent(contentBlock);
        m_keychain.sign(*data, signingWithSha256());
        genesisRecord.m_data = data;
        m_backend.putRecord(data);
        m_lastNames.push_back(genesisRecord.getRecordName());
    }
    NDN_LOG_INFO("STEP 2" << std::endl
                          << "- " << m_config.numGenesisBlock << " genesis records have been added to the DLedger");
    NDN_LOG_INFO("DLedger Initialization Succeed");
}

Mnemosyne::~Mnemosyne() {
    if (m_syncEventID) m_syncEventID.cancel();
}

ReturnCode
Mnemosyne::createRecord(Record &record) {
    NDN_LOG_INFO("[Mnemosyne::addRecord] Add new record");

    // randomly shuffle the tailing record list
    std::vector<Name> recordList = m_lastNames;
    std::shuffle(std::begin(recordList), std::end(recordList), m_randomEngine);

    for (const auto &tailRecord : recordList) {
        record.addPointer(tailRecord);
        if (record.getPointersFromHeader().size() >= m_config.precedingRecordNum)
            break;
    }

    auto data = make_shared<Data>(record.m_recordName);
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
    NDN_LOG_INFO("[Mnemosyne::addRecord] Added a new record:" << data->getFullName().toUri());

    // add new record into the ledger
    addRecord(data);

    //send sync interest
    auto rc = m_sync.publishData(data->wireEncode(), time::seconds(60));
    return ReturnCode::noError(data->getFullName().toUri());
}

optional<Record>
Mnemosyne::getRecord(const std::string &recordName) const {
    NDN_LOG_DEBUG("getRecord Called on " << recordName);
    return m_backend.getRecord(recordName);
}

bool
Mnemosyne::hasRecord(const std::string &recordName) const {
    auto dataPtr = m_backend.getRecord(Name(recordName));
    return dataPtr != nullptr;
}

std::list<Name>
Mnemosyne::listRecord(const std::string &prefix) const {
    return m_backend.listRecord(Name(prefix));
}

void Mnemosyne::onUpdate(const std::vector<ndn::svs::MissingDataInfo>& info) {

}

void Mnemosyne::addRecord(const shared_ptr<Data>& recordData) {
    m_backend.putRecord(recordData);
    m_lastNames[m_lastNameTops] = recordData->getFullName();
    m_lastNameTops ++;
    m_lastNameTops = m_lastNameTops % m_lastNames.size();
}

}  // namespace mnemosyne