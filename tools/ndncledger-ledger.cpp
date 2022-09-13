#include "ledger-module.hpp"

#include <boost/asio.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <chrono>
#include <deque>
#include <iostream>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>

namespace cledger::ledger {

static ndn::Face face;
static ndn::KeyChain keyChain;

static void
handleSignal(const boost::system::error_code& error, int signalNum)
{
  if (error) {
    return;
  }
  const char* signalName = ::strsignal(signalNum);
  std::cerr << "Exiting on signal ";
  if (signalName == nullptr) {
    std::cerr << signalNum;
  }
  else {
    std::cerr << signalName;
  }
  std::cerr << std::endl;
  face.getIoService().stop();
  exit(1);
}

static int
main(int argc, char* argv[])
{
  boost::asio::signal_set terminateSignals(face.getIoService());
  terminateSignals.add(SIGINT);
  terminateSignals.add(SIGTERM);
  terminateSignals.async_wait(handleSignal);

  std::string configFilePath(CLEDGER_SYSCONFDIR "/cledger/cledger.config");

  namespace po = boost::program_options;
  po::options_description optsDesc("Options");
  optsDesc.add_options()
  ("help,h", "print this help message and exit")
  ("config-file,c", po::value<std::string>(&configFilePath)->default_value(configFilePath), "path to configuration file");

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, optsDesc), vm);
    po::notify(vm);
  }
  catch (const po::error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }
  catch (const boost::bad_any_cast& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }

  if (vm.count("help") != 0) {
    std::cout << "Usage: " << argv[0] << " [options]\n"
              << "\n"
              << optsDesc;
    return 0;
  }

  LedgerModule ledger(face, keyChain, configFilePath);
  std::thread thread_reply([&ledger] {  
    while (true) {
      ledger.BackoffAndReply(std::chrono::seconds(10));
    }
  });
  face.processEvents();
  thread_reply.join();
  return 0;
}

} // namespace cledger::ledger

int
main(int argc, char* argv[])
{
  return cledger::ledger::main(argc, argv);
}