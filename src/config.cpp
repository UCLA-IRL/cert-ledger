#include "mnemosyne/config.hpp"
#include "default-cert-manager.h"

#include <ndn-cxx/util/io.hpp>
#include <utility>

namespace mnemosyne {

shared_ptr<Config>
Config::CustomizedConfig(const std::string &multicastPrefix, const std::string &interfacePrefix, const std::string &producerPrefix,
                         const std::string &anchorCertPath, const std::string &databasePath) {
    auto trustAnchorCert = io::load<security::Certificate>(anchorCertPath);
    if (trustAnchorCert == nullptr) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot load anchor certificate from the designated path."));
    }
    std::cout << "Trust Anchor: " << trustAnchorCert->getName().toUri() << std::endl;

    //starting peers
    std::list<security::Certificate> startingPeerCerts;
    auto config = std::make_shared<Config>(multicastPrefix, interfacePrefix, producerPrefix,
                                           make_shared<DefaultCertificateManager>(producerPrefix, trustAnchorCert,
                                                                                  startingPeerCerts));
    config->databasePath = databasePath;
    return config;
}

Config::Config(const std::string &syncPrefix, const std::string &interfacePrefix, const std::string &peerPrefix,
               shared_ptr<CertificateManager> certificateManager_)
        : syncPrefix(syncPrefix),
          interfacePrefix(interfacePrefix),
          peerPrefix(peerPrefix),
          certificateManager(std::move(certificateManager_)) {}

}  // namespace mnemosyne