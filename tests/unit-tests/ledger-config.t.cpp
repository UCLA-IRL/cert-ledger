#include "ledger-config.hpp"
#include "test-common.hpp"

namespace cledger::tests {
using ledger::LedgerConfig;

BOOST_FIXTURE_TEST_SUITE(TestConfig, IdentityManagementFixture)

BOOST_AUTO_TEST_CASE(LedgerConfigFile)
{
  LedgerConfig config;
  config.load("tests/unit-tests/config-files/config-ledger-1");
  BOOST_CHECK_EQUAL(config.ledgerPrefix, Name("/ndn/site1"));
  BOOST_CHECK_EQUAL(config.instanceSuffix, Name("/instance1"));
  BOOST_CHECK_EQUAL(config.nackFreshnessPeriod, time::seconds(10));
  BOOST_CHECK_EQUAL(config.recordZones.size(), 2);
  BOOST_CHECK_EQUAL(config.recordZones.front(), Name("/ndn/site1"));
  BOOST_CHECK_EQUAL(config.recordZones.back(), Name("/ndn/site2"));
  BOOST_CHECK_EQUAL(config.storageType, "storage-memory");
  BOOST_CHECK_EQUAL(config.policyType, "policy-descendants");
  BOOST_CHECK_EQUAL(config.policyThreshold, 3);
  BOOST_CHECK_EQUAL(config.interestSigner.getSignerType(), ndn::security::SigningInfo::SignerType::SIGNER_TYPE_HMAC);
  BOOST_CHECK_EQUAL(config.maxSegmentSize, 2000);
  BOOST_CHECK_EQUAL(config.sessionLength, ndn::time::seconds(60));
}

BOOST_AUTO_TEST_CASE(LedgerConfigFileWithErrors)
{
  LedgerConfig config;
  // nonexistent file
  BOOST_CHECK_THROW(config.load("tests/unit-tests/config-files/Nonexist"), std::runtime_error);
  // missing interlock policies
  BOOST_CHECK_THROW(config.load("tests/unit-tests/config-files/config-ledger-2"), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END() // TestConfig

} // namespace cledger::tests
