#ifndef CERT_LEDGER_INCLUDE_CONFIG_H_
#define CERT_LEDGER_INCLUDE_CONFIG_H_

#include <ndn-cxx/face.hpp>
#include <iostream>

using namespace ndn;
namespace cert_ledger {

class Config {
  public:

    static shared_ptr<Config> CustomizedConfig(const Name &multicastPrefix, const Name &peerPrefix,
                                               const std::string &databasePath);

    /**
     * Construct a Config instance used for CertLedger initialization.
     * @p multicastPrefix, input, the distributed ledger system's multicast prefix.
     * @p peerPrefix, input, the unique prefix of the peer.
     */
    Config(const Name &multicastPrefix, const Name &peerPrefix);

  public:
    /**
     * The number of preceding records that referenced by a later record.
     */
    size_t precedingRecordNum = 2;

    /**
     * The number of genesis block for the DAG.
     */
    size_t numGenesisBlock = 10;

    /**
     * The multicast prefix, under which an Interest can reach to all the peers in the same multicast group.
     */
    Name syncPrefix;
    /**
     * Producer's unique name prefix, under which an Interest can reach to the producer.
     */
    Name peerPrefix;
    /**
     * The path to the Database;
     */
    std::string databasePath;
};

} // namespace cert-ledger

#endif // define CERT_LEDGER_INCLUDE_CONFIG_H_