#include "cert-ledger/backend.hpp"
#include "ndn-cxx/security/signing-helpers.hpp"
#include "ndn-cxx/security/key-chain.hpp"
#include "cert-ledger/record.hpp"
#include <iostream>

using namespace cert_ledger;

ndn::security::KeyChain m_keyChain("pib-memory:", "tpm-memory:");

std::shared_ptr<ndn::Data>
makeData(const std::string& name, const std::string& content)
{
  using namespace ndn;
  using namespace std;
  Data insideData(name);
  insideData.setContent((const uint8_t*)content.c_str(), content.size());
  m_keyChain.sign(insideData, signingWithSha256());
  insideData.wireEncode();
  Record record("test", insideData);
  auto data = make_shared<Data>(record.getRecordName());
  m_keyChain.sign(*data, signingWithSha256());
  data->wireEncode();
  return data;
}

bool
testBackEnd()
{
  Backend backend("/tmp/test.leveldb");
  for (const auto &name : backend.listRecord("")) {
      backend.deleteRecord(name);
  }
  auto data = makeData("/cert-ledger/12345", "content is 12345");
  auto fullName = "/cert-ledger/12345";

  backend.putRecord(data);

  auto anotherRecord = backend.getRecord(fullName);
  if (data == nullptr || anotherRecord == nullptr) {
      return false;
  }
  return backend.listRecord(Name("/cert-ledger")).size() == 1 && data->wireEncode() == anotherRecord->wireEncode();
}

bool
testBackEndList() {
    Backend backend("/tmp/test-List.leveldb");
    for (const auto &name : backend.listRecord("")) {
        backend.deleteRecord(name);
    }
    for (int i = 0; i < 10; i++) {
        backend.putRecord(makeData("/cert-ledger/a/" + std::to_string(i), "content is " + std::to_string(i)));
        backend.putRecord(makeData("/cert-ledger/ab/" + std::to_string(i), "content is " + std::to_string(i)));
        backend.putRecord(makeData("/cert-ledger/b/" + std::to_string(i), "content is " + std::to_string(i)));
    }

    backend.putRecord(makeData("/cert-ledger/a", "content is "));
    backend.putRecord(makeData("/cert-ledger/ab", "content is "));
    backend.putRecord(makeData("/cert-ledger/b", "content is "));

    assert(backend.listRecord(Name("/cert-ledger")).size() == 33);
    assert(backend.listRecord(Name("/cert-ledger/a")).size() == 11);
    assert(backend.listRecord(Name("/cert-ledger/ab")).size() == 11);
    assert(backend.listRecord(Name("/cert-ledger/b")).size() == 11);
    assert(backend.listRecord(Name("/cert-ledger/a/5")).size() == 1);
    assert(backend.listRecord(Name("/cert-ledger/ab/5")).size() == 1);
    assert(backend.listRecord(Name("/cert-ledger/b/5")).size() == 1);
    assert(backend.listRecord(Name("/cert-ledger/a/55")).empty());
    assert(backend.listRecord(Name("/cert-ledger/ab/55")).empty());
    assert(backend.listRecord(Name("/cert-ledger/b/55")).empty());
    return true;
}

bool
testNameGet()
{
  std::string name1 = "name1";
  ndn::Name name2("/cert-ledger/name1/123");
  if (name2.get(-2).toUri() == name1) {
    return true;
  }
  return false;
}

int
main(int argc, char** argv)
{
  auto success = testBackEnd();
  if (!success) {
    std::cout << "testBackEnd failed" << std::endl;
  }
  else {
    std::cout << "testBackEnd with no errors" << std::endl;
  }
  success = testBackEndList();
  if (!success) {
    std::cout << "testBackEndList failed" << std::endl;
  }
  else {
    std::cout << "testBackEndList with no errors" << std::endl;
  }
  success = testNameGet();
  if (!success) {
    std::cout << "testNameGet failed" << std::endl;
  }
  else {
    std::cout << "testNameGet with no errors" << std::endl;
  }
  return 0;
}