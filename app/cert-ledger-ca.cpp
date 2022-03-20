// License: LGPL v3.0

#include "cert-ledger/cert-ledger.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <iostream>
#include <ndn-cxx/security/validator-config.hpp>

namespace po = boost::program_options;
using namespace ndn;
using namespace cert_ledger;

int main(int argc, char* argv[]) {

    po::options_description description("Usage for Cert_ledger Logger");

    description.add_options()
            ("help,h", "Display this help message")
            ("dag-sync-prefix,m", po::value<std::string>()->default_value("/ndn/broadcast/cert-ledger-dag"), "The prefix for DAG synchronization")
            ("interface-ps-prefix,i", po::value<std::string>()->default_value("/ndn/broadcast/cert-ledger"), "The prefix for Interface Pub/Sub")
            ("logger-prefix,l", po::value<std::string>(), "The prefix for the logger")
            ("trust-anchor,a", po::value<std::string>()->default_value("./cert-ledger-anchor.cert"), "The trust anchor file path for the logger")
            ("database-path,d", po::value<std::string>()->default_value("/tmp/cert-ledger-db/..."), "The database path for the logger");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    if(vm.count("help")){
        std::cout << description;
        return 0;
    }

    if (vm.count("logger-prefix") == 0) {
        std::cout << "missing parameter: Logger Prefix\n";
        return 2;
    }


    std::string identity = vm["logger-prefix"].as<std::string>();
    std::string databasePath = vm["database-path"].as<std::string>() == "/tmp/cert-ledger-db/..." ?
        std::string("/tmp/cert-ledger-db/" + identity.substr(identity.rfind('/'))) :
                               vm["database-path"].as<std::string>();

    boost::asio::io_service ioService;
    Face face(ioService);
    security::KeyChain keychain;
    std::shared_ptr<Config> config = nullptr;
    std::shared_ptr<ndn::security::Validator> caValidator;
    std::shared_ptr<ndn::security::Validator> loggerValidator;
    try {
        config = Config::CustomizedConfig(vm["dag-sync-prefix"].as<std::string>(),
                                          vm["interface-ps-prefix"].as<std::string>(),
                                          identity,
                                          databasePath
                                          );
        mkdir("/tmp/cert-ledger-db/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        auto configValidator = std::make_shared<ndn::security::ValidatorConfig>(face);
        configValidator->load("./test/loggers.schema");
        loggerValidator = configValidator;
        auto configValidator2 = std::make_shared<ndn::security::ValidatorConfig>(face);
        configValidator2->load("./test/clients.schema");
        caValidator = configValidator2;
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    auto ledger = std::make_shared<Cert_ledger>(*config, keychain, face, loggerValidator, caValidator);

    face.processEvents();
    return 0;
}
