//
// Created by Tyler on 1/15/22.
//

#include "util.hpp"

namespace util {

void KeyChainOptionSigner::sign(ndn::Interest& interest) const {
    m_keyChain.sign(interest, m_params);
}

void KeyChainOptionSigner::sign(ndn::Data& data) const {
    m_keyChain.sign(data, m_params);
}

}