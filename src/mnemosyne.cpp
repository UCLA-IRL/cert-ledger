#include "mnemosyne/mnemosyne.hpp"

#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/logging.hpp>

NDN_LOG_INIT(mnemosyne.impl);

using namespace ndn;
namespace mnemosyne {


Mnemosyne::Mnemosyne(const Config &config, KeyChain &keychain, Face &network) :
        MnemosyneDagSync(config, keychain, network),
        m_interfacePS(config.interfacePrefix, config.peerPrefix, network, [&](const auto& i){onInterfaceUpdate(i);})
{

}

void Mnemosyne::onInterfaceUpdate(const std::vector<ndn::svs::MissingDataInfo> &info) {

}

Mnemosyne::~Mnemosyne() = default;

}  // namespace mnemosyne