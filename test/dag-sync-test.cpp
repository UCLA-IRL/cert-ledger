#include "cert-ledger/record.hpp"
#include "cert-ledger/cert-ledger.hpp"
#include <iostream>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/security/validator.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <boost/asio/io_service.hpp>
#include <ndn-cxx/util/io.hpp>
#include <random>

using namespace cert_ledger;

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 random_gen(rd()); //Standard mersenne_twister_engine seeded with rd()

void periodicAddRecord(KeyChain& keychain, shared_ptr<Cert_ledgerDagSync> ledger, Scheduler &scheduler) {
    std::uniform_int_distribution<int> distribution(0, INT_MAX);
    Data data("/a/b/" + std::to_string(distribution(random_gen)));
    data.setContent(makeStringBlock(tlv::Content, std::to_string(distribution(random_gen))));
    keychain.sign(data, signingWithSha256());

    Record record(ledger->getPeerPrefix(), data);
    ledger->createRecord(record);

    // schedule for the next record generation
    scheduler.schedule(time::seconds(5), [&keychain, ledger, &scheduler] { periodicAddRecord(keychain, ledger, scheduler); });
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s identity\n", argv[0]);
        return 1;
    }
    std::string identity = argv[1];
    boost::asio::io_service ioService;
    Face face(ioService);
    security::KeyChain keychain;
    std::shared_ptr<Config> config = nullptr;
    std::shared_ptr<ndn::security::Validator> validator;
    try {
        config = Config::CustomizedConfig("/ndn/broadcast/cert-ledger-dag", "/ndn/broadcast/cert-ledger", identity,
                                          std::string("/tmp/cert-ledger-db/" + identity.substr(identity.rfind('/'))));
        auto configValidator = std::make_shared<ndn::security::ValidatorConfig>(face);
        configValidator->load("./test/loggers.schema");
        validator = configValidator;
        mkdir("/tmp/cert-ledger-db/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    auto ledger = std::make_shared<Cert_ledgerDagSync>(*config, keychain, face, validator);

    Scheduler scheduler(ioService);
    periodicAddRecord(keychain, ledger, scheduler);

    face.processEvents();
    return 0;
}