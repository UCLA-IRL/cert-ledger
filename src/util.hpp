//
// Created by Tyler on 1/15/22.
//

#ifndef MNEMOSYNE_UTIL_HPP
#define MNEMOSYNE_UTIL_HPP


#include <ndn-svs/security-options.hpp>
#include <memory>
#include <utility>

namespace util {

/**
 * A adaptation class from cxx validator to svs validator
 */
class cxxValidator : public ndn::svs::BaseValidator {
  public:

    cxxValidator(std::shared_ptr<ndn::security::Validator> validator) :
    m_validator(std::move(validator)){

    }

    /**
     * @brief Asynchronously validate @p data
     *
     * @note @p successCb and @p failureCb must not be nullptr
     */
    virtual void
    validate(const ndn::Data &data,
             const ndn::security::DataValidationSuccessCallback &successCb,
             const ndn::security::DataValidationFailureCallback &failureCb) {
        m_validator->validate(data, successCb, failureCb);
    }

    /**
     * @brief Asynchronously validate @p interest
     *
     * @note @p successCb and @p failureCb must not be nullptr
     */
    virtual void
    validate(const ndn::Interest &interest,
             const ndn::security::InterestValidationSuccessCallback &successCb,
             const ndn::security::InterestValidationFailureCallback &failureCb) {
        m_validator->validate(interest, successCb, failureCb);
    }

    virtual ~cxxValidator() = default;

  private:
    std::shared_ptr<ndn::security::Validator> m_validator;
};

/**
 * A adaptation class from cxx validator to svs validator
 */
class alwaysFailValidator : public ndn::svs::BaseValidator {
  public:

    alwaysFailValidator() {

    }

    /**
     * @brief Asynchronously validate @p data
     *
     * @note @p successCb and @p failureCb must not be nullptr
     */
    virtual void
    validate(const ndn::Data &data,
             const ndn::security::DataValidationSuccessCallback &successCb,
             const ndn::security::DataValidationFailureCallback &failureCb) {
        failureCb(data, ndn::security::ValidationError(ndn::security::ValidationError::Code::INVALID_SIGNATURE, "No verification allowed"));
    }

    /**
     * @brief Asynchronously validate @p interest
     *
     * @note @p successCb and @p failureCb must not be nullptr
     */
    virtual void
    validate(const ndn::Interest &interest,
             const ndn::security::InterestValidationSuccessCallback &successCb,
             const ndn::security::InterestValidationFailureCallback &failureCb) {
        failureCb(interest, ndn::security::ValidationError(ndn::security::ValidationError::Code::INVALID_SIGNATURE, "No verification allowed"));
    }

    virtual ~alwaysFailValidator() = default;
};

/**
 * A signer using an ndn-cxx keychain instance
 */
class KeyChainOptionSigner : public BaseSigner {
  public:
    KeyChainOptionSigner(ndn::KeyChain& keyChain, ndn::security::SigningInfo params) : m_keyChain(keyChain), m_params(std::move(params)) {}

    void
    sign(ndn::Interest& interest) const;

    void
    sign(ndn::Data& data) const;

    virtual ~KeyChainOptionSigner() = default;
  private:
    ndn::KeyChain& m_keyChain;
    ndn::security::SigningInfo m_params;
};

}


#endif //MNEMOSYNE_UTIL_HPP
