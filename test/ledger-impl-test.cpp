#include "mnemosyne/record.hpp"
#include "mnemosyne/mnemosyne.hpp"
#include <iostream>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <boost/asio/io_service.hpp>
#include <ndn-cxx/util/io.hpp>
#include <random>

using namespace mnemosyne;

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 random_gen(rd()); //Standard mersenne_twister_engine seeded with rd()

void periodicAddRecord(KeyChain& keychain, shared_ptr<Mnemosyne> ledger, Scheduler &scheduler) {
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
    try {
        config = Config::CustomizedConfig("/ndn/broadcast/mnemosyne", identity,
                                          std::string("./test/mnemosyne-anchor.cert"),
                                          std::string("/tmp/mnemosyne-db/" + identity.substr(identity.rfind('/'))));
        mkdir("/tmp/mnemosyne-db/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    shared_ptr<Mnemosyne> ledger = std::make_shared<Mnemosyne>(*config, keychain, face);

    Scheduler scheduler(ioService);
    periodicAddRecord(keychain, ledger, scheduler);

    face.processEvents();
    return 0;
}