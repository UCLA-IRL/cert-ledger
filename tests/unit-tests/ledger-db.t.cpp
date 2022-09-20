#include "storage/ledger-leveldb.hpp"
#include "test-common.hpp"

namespace cledger::tests {

using namespace cledger::storage;

BOOST_FIXTURE_TEST_SUITE(TestLevelDBStorage, IdentityManagementTimeFixture)

BOOST_AUTO_TEST_CASE(LevelDBWorkflow)
{
  LedgerLevelDB storage(Name("/ndn/ledger1"), ".test_db");
  Data data(Name("/ndn/site1"));
  ndn::security::SigningInfo info;
  info.setSha256Signing();
  m_keyChain.sign(data,info);

  // add operation
  BOOST_CHECK_NO_THROW(storage.addBlock(data.getName(), data.wireEncode()));

  // get operation
  Block res;
  BOOST_CHECK_NO_THROW(res = storage.getBlock(data.getName()));
  BOOST_CHECK_EQUAL(data.wireEncode(), res);

  // delete operation
  BOOST_CHECK_NO_THROW(storage.deleteBlock(data.getName()));
}

BOOST_AUTO_TEST_SUITE_END() // TestDBStorage

} // namespace cledger::tests