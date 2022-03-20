#ifndef CERT_LEDGER_BACKEND_H_
#define CERT_LEDGER_BACKEND_H_

#include <ndn-cxx/data.hpp>
#include <leveldb/db.h>

using namespace ndn;
namespace cert_ledger {

class Backend {
  public:
    Backend(const std::string &dbDir);

  public:
    ~Backend();

    // @param the recordName must be a full name (i.e., containing explicit digest component)
    shared_ptr<Data>
    getRecord(const Name &recordName) const;

    bool
    putRecord(const shared_ptr<const Data> &recordData);

    void
    deleteRecord(const Name &recordName);

    std::list<Name>
    listRecord(const Name &prefix) const;

  private:
    leveldb::DB *m_db;
};

}  // namespace cert-ledger

#endif  // CERT_LEDGER_BACKEND_H_