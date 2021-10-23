#ifndef MNEMOSYNE_MNEMOSYNE_H_
#define MNEMOSYNE_MNEMOSYNE_H_

#include "mnemosyne/mnemosyne.hpp"
#include "mnemosyne/record.hpp"
#include "mnemosyne/config.hpp"
#include "mnemosyne/return-code.hpp"
#include "backend.hpp"
#include <ndn-svs/svsync.hpp>
#include <ndn-cxx/security/certificate.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <boost/asio/io_service.hpp>
#include <ndn-cxx/util/io.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <stack>
#include <random>


using namespace ndn;
namespace mnemosyne {

class Mnemosyne {
  public:
    /**
   * Initialize a DLedger instance from the config.
   * @p config, input, the configuration of multicast prefix, peer prefix, and settings of Dledger behavior
   * @p keychain, input, the local NDN keychain instance
   * @p face, input, the localhost NDN face to send/receive NDN packets.
   */
    Mnemosyne(const Config &config, security::KeyChain &keychain, Face &network);

    virtual ~Mnemosyne();

    /**
     * Create a new record to the Dledger.
     * @p record, input, a record instance which contains the record payload
     */
    virtual ReturnCode
    createRecord(Record &record);

    /**
     * Get an existing record from the Dledger.
     * @p recordName, input, the name of the record, which is an NDN full name (i.e., containing ImplicitSha256DigestComponent component)
     */
    virtual optional<Record>
    getRecord(const std::string &recordName) const;

    /**
     * Check whether the record exists in the Dledger.
     * @p recordName, input, the name of the record, which is an NDN full name (i.e., containing ImplicitSha256DigestComponent component)
     */
    virtual bool
    hasRecord(const std::string &recordName) const;

    /**
      * list the record exists in the Dledger.
      * @p recordName, input, the name of the record, which is an NDN name prefix.
      */
    virtual std::list<Name>
    listRecord(const std::string &prefix) const;

  private:
    void onUpdate(const std::vector<ndn::svs::MissingDataInfo>& info);

    void addRecord(const shared_ptr<Data>& recordData);

  private:
    Config m_config;
    Face &m_network;
    Scheduler m_scheduler;
    Backend m_backend;
    security::KeyChain &m_keychain;
    svs::SVSync m_sync;

    ndn::scheduler::EventId m_syncEventID;
    std::vector<Name> m_lastNames;
    int m_lastNameTops;

    std::mt19937_64 m_randomEngine;
  };

} // namespace mnemosyne

#endif // MNEMOSYNE_MNEMOSYNE_H_