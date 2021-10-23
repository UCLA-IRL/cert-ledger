#ifndef MNEMOSYNE_INCLUDE_CONFIG_H_
#define MNEMOSYNE_INCLUDE_CONFIG_H_

#include "cert-manager.hpp"
#include <ndn-cxx/face.hpp>
#include <iostream>

using namespace ndn;
namespace mnemosyne {

class Config {
  public:

    static shared_ptr<Config> CustomizedConfig(const std::string &multicastPrefix, const std::string &interfacePrefix, const std::string &peerPrefix,
                                               const std::string &anchorCertPath, const std::string &databasePath);

    /**
     * Construct a Config instance used for Mnemosyne initialization.
     * @p multicastPrefix, input, the distributed ledger system's multicast prefix.
     * @p peerPrefix, input, the unique prefix of the peer.
     */
    Config(const std::string &multicastPrefix, const std::string &interfacePrefix, const std::string &peerPrefix,
           shared_ptr<CertificateManager> certificateManager_);

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
     * The timeout for fetching ancestor records.
     */
    time::milliseconds ancestorFetchTimeout = time::milliseconds(10000);

    /**
     * The multicast prefix, under which an Interest can reach to all the peers in the same multicast group.
     */
    Name syncPrefix;
    /**
     * The interface pub/sub prefix, under which an publication can reach all Mnemosyne loggers.
     */
    Name interfacePrefix;
    /**
     * Producer's unique name prefix, under which an Interest can reach to the producer.
     */
    Name peerPrefix;
    /**
     * The path to the Database;
     */
    std::string databasePath;
    /**
     * The Certificate manager
     */
    shared_ptr<CertificateManager> certificateManager;
};

} // namespace mnemosyne

#endif // define MNEMOSYNE_INCLUDE_CONFIG_H_