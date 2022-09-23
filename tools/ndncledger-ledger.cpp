#include "ledger-module.hpp"

#include <boost/asio.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <chrono>
#include <deque>
#include <iostream>
#include <experimental/random>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/scheduler.hpp>

namespace cledger::ledger {

static ndn::Face face;
static ndn::Scheduler scheduler{face.getIoService()};
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

static void
scheduleWrapper(time::seconds backoffPeriod, ndn::scheduler::EventCallback eventCb)
{
  auto randomized = std::experimental::randint(100, 110) / 100 * backoffPeriod;
  scheduler.schedule(randomized, [backoffPeriod, eventCb] {
    scheduleWrapper(backoffPeriod, eventCb);
    eventCb();
  });
}
static int
main(int argc, char* argv[])
{
  boost::asio::signal_set terminateSignals(face.getIoService());
  terminateSignals.add(SIGINT);
  terminateSignals.add(SIGTERM);
  terminateSignals.async_wait(handleSignal);

  std::string configFilePath(CLEDGER_SYSCONFDIR "/cledger/cledger.config");
  std::string backoffPeriodStr(std::to_string(3600));

  namespace po = boost::program_options;
  po::options_description optsDesc("Options");
  optsDesc.add_options()
  ("help,h", "print this help message and exit")
  ("config-file,c", po::value<std::string>(&configFilePath)->default_value(configFilePath), "path to configuration file")
  ("backoff-period,b", po::value<std::string>(&backoffPeriodStr)->default_value(backoffPeriodStr), "backoff period (in seconds) of generating reply record");

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
  auto backoffPeriod = time::seconds(std::stoul(backoffPeriodStr));
  scheduleWrapper(backoffPeriod, [&ledger] {ledger.publishReply();});
  face.processEvents();
  return 0;
}

} // namespace cledger::ledger

int
main(int argc, char* argv[])
{
  return cledger::ledger::main(argc, argv);
}