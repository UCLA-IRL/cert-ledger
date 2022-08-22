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

LedgerModule::LedgerModule(ndn::Face& face, ndn::KeyChain& keyChain, const std::string& configPath, const std::string& storageType)
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
  m_dag = std::make_unique<dag::DagModule>(m_storage->getInterface());
  m_dag->setInterlockPolicy(std::make_shared<dag::InterlockPolicyDescendants>());

  // initialize sync module
  sync::SyncOptions syncOps;
  sync::SecurityOptions secOps(m_keyChain);
  syncOps.prefix = m_config.ledgerPrefix;
  syncOps.id = m_config.instanceSuffix;
  // only for now
  secOps.interestSigner->signingInfo.setSigningHmacKey("certledger2022demo");
  secOps.dataSigner->signingInfo.setSigningHmacKey("certledger2022demo");
  // a wrapper for record finder
  m_sync = std::make_unique<sync::SyncModule>(syncOps, secOps, m_face,
    m_storage->getInterface(),
    [this] (const Record& record) {
      m_dag->add(record);
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
} // namespace cledger::ledger
