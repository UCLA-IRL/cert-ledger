#ifndef MNEMOSYNE_MNEMOSYNE_DAG_SYNC_H_
#define MNEMOSYNE_MNEMOSYNE_DAG_SYNC_H_

#include <ndn-svs/svsync.hpp>
#include "record.hpp"
#include "config.hpp"
#include "return-code.hpp"
#include "backend.hpp"
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/io.hpp>
#include <stack>
#include <random>
#include <utility>


using namespace ndn;
namespace mnemosyne {

class MnemosyneDagSync {
  public:
    /**
   * Initialize a MnemosyneDagSync instance from the config.
   * @p config, input, the configuration of multicast prefix, peer prefix, and settings of Dledger behavior
   * @p keychain, input, the local NDN keychain instance
   * @p face, input, the localhost NDN face to send/receive NDN packets.
   */
    MnemosyneDagSync(const Config &config, security::KeyChain &keychain, Face &network, std::shared_ptr<ndn::security::Validator> m_recordValidator);

    virtual ~MnemosyneDagSync();

    /**
     * Create a new record to the MnemosyneDagSync.
     * @p record, input, a record instance which contains the record payload
     */
    virtual ReturnCode
    createRecord(Record &record);

    /**
     * Get an existing record from the MnemosyneDagSync.
     * @p recordName, input, the name of the record, which is an NDN full name (i.e., containing ImplicitSha256DigestComponent component)
     */
    virtual optional<Record>
    getRecord(const std::string &recordName) const;

    /**
     * Check whether the record exists in the MnemosyneDagSync.
     * @p recordName, input, the name of the record, which is an NDN full name (i.e., containing ImplicitSha256DigestComponent component)
     */
    virtual bool
    hasRecord(const std::string &recordName) const;

    /**
      * list the record exists in the MnemosyneDagSync.
      * @p recordName, input, the name of the record, which is an NDN name prefix.
      */
    virtual std::list<Name>
    listRecord(const std::string &prefix) const;

    const Name& getPeerPrefix() const;

    void setOnRecordCallback(std::function<void(const Record&)> callback) {
        m_onRecordCallback = std::move(callback);
    }

  private:
    void onUpdate(const std::vector<ndn::svs::MissingDataInfo>& info);

    void addReceivedRecord(const shared_ptr<Data>& recordData);

    ndn::svs::SecurityOptions getSecurityOption();

  protected:
    const Config m_config;
    Backend m_backend;
    security::KeyChain &m_keychain;
    svs::SVSync m_dagSync;
    std::shared_ptr<ndn::security::Validator> m_recordValidator;
    std::function<void(const Record&)> m_onRecordCallback;

    std::vector<Name> m_lastNames;
    unsigned int m_lastNameTops;
    Name m_selfLastName;

    std::mt19937_64 m_randomEngine;

    void addSelfRecord(const shared_ptr<Data> &data);
};

} // namespace mnemosyne

#endif // MNEMOSYNE_MNEMOSYNE_DAG_SYNC_H_