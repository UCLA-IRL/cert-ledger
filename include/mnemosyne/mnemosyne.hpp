#ifndef MNEMOSYNE_MNEMOSYNE_H_
#define MNEMOSYNE_MNEMOSYNE_H_

#include "mnemosyne/mnemosyne-dag-sync.hpp"

using namespace ndn;
namespace mnemosyne {

class Mnemosyne : public MnemosyneDagSync {
  public:
    /**
   * Initialize a Mnemosyne instance from the config.
   * @p config, input, the configuration of multicast prefix, peer prefix, and settings of Dledger behavior
   * @p keychain, input, the local NDN keychain instance
   * @p face, input, the localhost NDN face to send/receive NDN packets.
   */
    Mnemosyne(const Config &config, security::KeyChain &keychain, Face &network);

    virtual ~Mnemosyne();


  private:

  };

} // namespace mnemosyne

#endif // MNEMOSYNE_MNEMOSYNE_H_