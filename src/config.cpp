#include "cert-ledger/config.hpp"

#include <ndn-cxx/util/io.hpp>
#include <utility>

namespace cert_ledger {

shared_ptr<Config>
Config::CustomizedConfig(const Name &multicastPrefix, const Name &producerPrefix,
                         const std::string &databasePath) {

    //starting peers
    std::list<security::Certificate> startingPeerCerts;
    auto config = std::make_shared<Config>(multicastPrefix, producerPrefix);
    config->databasePath = databasePath;
    return config;
}

Config::Config(const Name &syncPrefix, const Name &peerPrefix)
        : syncPrefix(syncPrefix),
          peerPrefix(peerPrefix) {}

}  // namespace cert-ledger