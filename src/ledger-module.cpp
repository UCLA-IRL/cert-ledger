#include "ledger-module.hpp"
#include "nack.hpp"
#include "dag/interlock-policy-descendants.hpp"

#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/security/verification-helpers.hpp>
#include <ndn-cxx/util/io.hpp>
#include <ndn-cxx/util/random.hpp>
#include <ndn-cxx/util/string-helper.hpp>

namespace cledger::ledger {

NDN_LOG_INIT(cledger.ledger);

LedgerModule::LedgerModule(ndn::Face& face, ndn::KeyChain& keyChain, const std::string& configPath,
                           const std::string& storageType, const std::string& policyType)
  : m_face(face)
  , m_keyChain(keyChain)
{
  // load the config and create storage
  m_config.load(configPath);
  m_validator.load(m_config.schemaFile);
  registerPrefix();
  
  Name topic = Name(m_config.ledgerPrefix).append("LEDGER").append("append");

  // initiliaze CA facing prefixes
  m_appendCt = std::make_unique<append::Ledger>(m_config.ledgerPrefix, topic, m_face, m_keyChain, m_validator);
  m_appendCt->listen(std::bind(&LedgerModule::onDataSubmission, this, _1));

  // initialize backend storage module
  m_storage = storage::LedgerStorage::createLedgerStorage(storageType, m_config.ledgerPrefix, "");

  // dag engine
  m_policy = dag::policy::InterlockPolicy::createInterlockPolicy(policyType, "");
  m_dag = std::make_unique<dag::DagModule>(m_storage->getInterface(), m_policy->getInterface());

  // initialize sync module
  sync::SyncOptions syncOps;
  sync::SecurityOptions secOps(m_keyChain);
  syncOps.prefix = m_config.ledgerPrefix;
  syncOps.id = m_config.instanceSuffix;
  // only for now
  secOps.interestSigner->signingInfo.setSigningHmacKey("certledger2022demo");
  secOps.dataSigner->signingInfo.setSigningHmacKey("certledger2022demo");
  m_sync = std::make_unique<sync::SyncModule>(syncOps, secOps, m_face,
    m_storage->getInterface(),
    [this] (const Record& record) {
      m_dag->add(record);
      printDagChanges();
    }
  );  
}

void
LedgerModule::registerPrefix()
{
  // register prefixes
  Name prefix = m_config.ledgerPrefix;
  // let's first use "LEDGER" in protocol
  prefix.append("LEDGER");

  auto prefixId = m_face.registerPrefix(
    prefix,
    [&] (const Name& name) {
      // register for each record Zone
      // notice: this only register FIB to Face, not NFD.
      for (auto& zone : m_config.recordZones) {
        auto filterId = m_face.setInterestFilter(zone, [this] (auto&&, const auto& i) { onQuery(i); });
        NDN_LOG_TRACE("Registering filter for recordZone " << zone);
        m_handle.handleFilter(filterId);
      }
    },
    [this] (auto&&, const auto& reason) { onRegisterFailed(reason); }
  );
  m_handle.handlePrefix(prefixId);
}

AppendStatus 
LedgerModule::onDataSubmission(const Data& data)
{
  NDN_LOG_TRACE("Received Submission " << data);
  AppendStatus ret;
  m_validator.validate(data,
    [this, &data, &ret] (const Data&) {
      NDN_LOG_TRACE("Submitted Data conforms to trust schema");
      try {
        m_storage->addBlock(data.getName(), data.wireEncode());
        ret = AppendStatus::SUCCESS;
      }
      catch (std::exception& e) {
        NDN_LOG_TRACE("Submission failed because of: " << e.what());
        ret = AppendStatus::FAILURE_STORAGE;
      }
    },
    [&ret] (const Data&, const ndn::security::ValidationError& error) {
      NDN_LOG_ERROR("Error authenticating data: " << error);
      ret = AppendStatus::FAILURE_VALIDATION_APP;
    });
  return ret;
}

void
LedgerModule::onQuery(const Interest& query) {
  // need to validate query format
  if (query.getForwardingHint().empty()) {
    // non-related, discard
    return;
  }

  NDN_LOG_TRACE("Received Query " << query);
  try {
    Data data(m_storage->getBlock(query.getName()));
    NDN_LOG_TRACE("Ledger replies with: " << data.getName());
    m_face.put(data);
  }
  catch (std::exception& e) {
    NDN_LOG_DEBUG("Ledger storage cannot get the Data for reason: " << e.what());
    // reply with app layer nack
    Nack nack;
    auto data = nack.prepareData(query.getName(), time::toUnixTimestamp(time::system_clock::now()));
    data->setFreshnessPeriod(m_config.nackFreshnessPeriod);
    m_keyChain.sign(*data, signingByIdentity(m_config.ledgerPrefix));
    NDN_LOG_TRACE("Ledger replies with: " << data->getName());
    m_face.put(*data);
  }
}

void
LedgerModule::onRegisterFailed(const std::string& reason)
{
  NDN_LOG_ERROR("Failed to register prefix in local hub's daemon, REASON: " << reason);
}

void
LedgerModule::afterValidation(const Data& data)
{
  NDN_LOG_TRACE("Receiving validated Data " << data.getName());
  // put raw data into storage
  m_storage->addBlock(data.getName(), data.wireEncode());

  // it is either a certificate or revocation record
  Record newRecord;
  std::list<Name> pointers;
  // name is given by SVS, don't set it

  auto dataTlv = data.wireEncode();
  newRecord.setPayload(make_span<const uint8_t>(dataTlv.wire(), dataTlv.size()));
  for (auto& i : m_dag->getWaitList(0)) {
    NDN_LOG_DEBUG("Referencing to [Generic] " << dag::fromStateName(i));
    pointers.push_back(dag::fromStateName(i));
  }
  
  if (pointers.size() < 1) {
    // no waitlist, make a gensis record only referencing to itself
    newRecord.setType(tlv::GENESIS_RECORD);
    // a gensis record must be its first SVS publication,
    // therefore we can guess the name
    auto genesisName = m_sync->getSyncBase()->getMyDataName(1);
    NDN_LOG_DEBUG("Referencing to [Genesis] " << genesisName);
    newRecord.setPointers({genesisName});
  }
  else {
    // a generic record, add pointers normally
    newRecord.setType(tlv::GENERIC_RECORD);
    newRecord.setPointers(pointers);
  }
  // publish record
  Name name = m_sync->publishRecord(newRecord);
  newRecord.setName(name);
  // add to DAG
  m_dag->add(newRecord);
  NDN_LOG_DEBUG("Generating new Record " << newRecord.getName());
  printDagChanges();
}

void
LedgerModule::printDagChanges()
{
  // harvest record that collects enough citations (e.g., 3)
  auto recordList = m_dag->harvest(2);
  // put those record into 
  if (recordList.size() > 0) {
    NDN_LOG_DEBUG("The following Records have been interlocked");
    for (auto& i : recordList) {
      NDN_LOG_DEBUG("   " << i.getName());
    }
  }
}
} // namespace cledger::ledger
