//
// Created by Tyler on 8/8/20.
//

#include <iostream>
#include <utility>
#include <ndn-cxx/security/verification-helpers.hpp>
#include "default-cert-manager.h"

mnemosyne::DefaultCertificateManager::DefaultCertificateManager(const Name &peerPrefix,
                                                                shared_ptr<security::Certificate> anchorCert,
                                                                const std::list<security::Certificate> &startingPeers)
        :
        m_peerPrefix(peerPrefix), m_anchorCert(std::move(anchorCert)) {
    if (!m_anchorCert->isValid()) {
        BOOST_THROW_EXCEPTION(std::runtime_error("trust Anchor Expired"));
    }
    m_peerCertificates[m_anchorCert->getIdentity()].push_back(*m_anchorCert);
    for (const auto &certificate: startingPeers) {
        m_peerCertificates[certificate.getIdentity()].push_back(certificate);
    }
}

bool mnemosyne::DefaultCertificateManager::verifySignature(const Data &data) const {
    auto identity = Record(data).getProducerPrefix();
    auto iterator = m_peerCertificates.find(identity);
    if (iterator == m_peerCertificates.cend()) return false;
    for (const auto &cert : iterator->second) {
        if (security::verifySignature(data, cert)) {
            return true;
        }
    }
    return false;
}

bool mnemosyne::DefaultCertificateManager::verifyRecordFormat(const mnemosyne::Record &record) const {

    return true;
}

bool mnemosyne::DefaultCertificateManager::endorseSignature(const Data &data) const {
    auto identity = Record(data).getProducerPrefix();
    auto iterator = m_peerCertificates.find(identity);
    if (iterator == m_peerCertificates.cend()) return false;
    for (const auto &cert : iterator->second) {
        if (m_revokedCertificates.count(cert.getFullName())) continue;
        if (security::verifySignature(data, cert)) {
            return true;
        }
    }
    return false;
}

bool mnemosyne::DefaultCertificateManager::verifySignature(const Interest &interest) const {
    SignatureInfo info(interest.getName().get(-2).blockFromValue());
    auto identity = info.getKeyLocator().getName().getPrefix(-2);
    auto iterator = m_peerCertificates.find(identity);
    if (iterator == m_peerCertificates.cend()) return false;
    for (const auto &cert : iterator->second) {
        if (m_revokedCertificates.count(cert.getFullName())) continue;
        if (security::verifySignature(interest, cert)) {
            return true;
        }
    }
    return false;
}

void mnemosyne::DefaultCertificateManager::acceptRecord(const mnemosyne::Record &record) {

}

Name mnemosyne::DefaultCertificateManager::getCertificateNameIdentity(const Name &certificateName) const {
    if (certificateName.get(-1).isImplicitSha256Digest())
        return certificateName.getPrefix(-1)
                .getPrefix(security::Certificate::KEY_COMPONENT_OFFSET); // remove another component from back
    else
        return certificateName.getPrefix(security::Certificate::KEY_COMPONENT_OFFSET);
}

bool mnemosyne::DefaultCertificateManager::authorizedToGenerate() const {
    auto iterator = m_peerCertificates.find(m_peerPrefix);
    if (iterator == m_peerCertificates.cend()) return false;
    return !iterator->second.empty();
}
