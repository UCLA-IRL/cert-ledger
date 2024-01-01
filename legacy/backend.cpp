#include "cert-ledger/backend.hpp"
#include "cert-ledger/record.hpp"

#include <cassert>
#include <iostream>

namespace cert_ledger {

Backend::Backend(const std::string &dbDir) {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, dbDir, &m_db);
    if (!status.ok()) {
        std::cerr << "Unable to open/create database " << dbDir << std::endl;
        std::cerr << status.ToString() << std::endl;
        BOOST_THROW_EXCEPTION(std::runtime_error("Unable to open/create database"));
    }
}

Backend::~Backend() {
    delete m_db;
}

shared_ptr<Data>
Backend::getRecord(const Name &contentName) const {
    const auto &nameStr = contentName.toUri();
    leveldb::Slice key = nameStr;
    std::string value;
    leveldb::Status s = m_db->Get(leveldb::ReadOptions(), key, &value);
    if (!s.ok()) {
        return nullptr;
    } else {
        ndn::Block block(span<const uint8_t>{reinterpret_cast<const uint8_t *>(value.c_str()), value.size()});
        return make_shared<Data>(block);
    }
}

bool
Backend::putRecord(shared_ptr<const Data> recordData) {
    const auto &nameStr = Record::getContentName(recordData->getName()).toUri();
    leveldb::Slice key = nameStr;
    auto recordBytes = recordData->wireEncode();
    leveldb::Slice value((const char *)recordBytes.data(), recordBytes.size());
    leveldb::Status s = m_db->Put(leveldb::WriteOptions(), key, value);
    if (!s.ok()) {
        return false;
    }
    return true;
}

void
Backend::deleteRecord(const Name &contentName) {
    const auto &nameStr = contentName.toUri();
    leveldb::Slice key = nameStr;
    leveldb::Status s = m_db->Delete(leveldb::WriteOptions(), key);
    if (!s.ok()) {
        std::cerr << "Unable to delete value from database, key: " << nameStr << std::endl;
        std::cerr << s.ToString() << std::endl;
    }
}

std::list<Name>
Backend::listRecord(const Name &prefix) const {
    std::list<Name> names;
    leveldb::Iterator *it = m_db->NewIterator(leveldb::ReadOptions());
    for (it->Seek(prefix.toUri()); it->Valid() && prefix.isPrefixOf(Name(it->key().ToString())); it->Next()) {
        names.emplace_back(it->key().ToString());
    }
    assert(it->status().ok());  // Check for any errors found during the scan
    delete it;
    return std::move(names);
}

}  // namespace cert-ledger