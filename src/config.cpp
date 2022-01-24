#include "mnemosyne/config.hpp"
#include "default-cert-manager.h"

#include <ndn-cxx/util/io.hpp>
#include <utility>

namespace mnemosyne {

shared_ptr<Config>
Config::CustomizedConfig(const std::string &multicastPrefix, const std::string &interfacePrefix, const std::string &producerPrefix,
                         const std::string &databasePath) {

    //starting peers
    std::list<security::Certificate> startingPeerCerts;
    auto config = std::make_shared<Config>(multicastPrefix, interfacePrefix, producerPrefix);
    config->databasePath = databasePath;
    return config;
}

Config::Config(const std::string &syncPrefix, const std::string &interfacePrefix, const std::string &peerPrefix)
        : syncPrefix(syncPrefix),
          interfacePrefix(interfacePrefix),
          peerPrefix(peerPrefix) {}

}  // namespace mnemosyne