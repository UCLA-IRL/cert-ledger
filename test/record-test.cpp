#include "mnemosyne/record.hpp"
#include "mnemosyne/mnemosyne.hpp"
#include <iostream>

#include <ndn-cxx/util/io.hpp>

using namespace mnemosyne;

int main(int argc, char const *argv[]) {
    std::shared_ptr<Config> config = nullptr;
    try {
        config = Config::DefaultConfig();
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    security::KeyChain keychain;
    Face face;
    Mnemosyne ledger(*config, keychain, face);
    return 0;
}
