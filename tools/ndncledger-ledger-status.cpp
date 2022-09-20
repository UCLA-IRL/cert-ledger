#include "ledger-module.hpp"
#include "dag/edge-state-list.hpp"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <iostream>

namespace cledger::ledger {

static int
main(int argc, char* argv[])
{
  namespace po = boost::program_options;
  std::string ledgerConfigString = "";
  bool isBenchmark = false;
  po::options_description description(
    "Usage: ndncledger-ledger-status [-h] -c ledgerConfig\n"
    "\n"
    "Options");
  description.add_options()
    ("help,h", "produce help message")
    ("ledgerConfig,c",  po::value<std::string>(&ledgerConfigString),
                         "ledger config file name (e.g., ledger.conf.sample)")
    ("benchmark,b",     po::bool_switch(&isBenchmark), "only print interlock latency");
  po::positional_options_description p;
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(description).positional(p).run(), vm);
    po::notify(vm);
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
  if (vm.count("help") != 0) {
    std::cerr << description << std::endl;
    return 0;
  }
  if (vm.count("ledgerConfig") == 0) {
    std::cerr << "ERROR: you must specify a ledger configuration." << std::endl;
    return 2;
  }

  LedgerConfig config;
  config.load(ledgerConfigString);
  // access to corresponding memory
  if (config.storageType == "storage-memory") {
    std::cerr << "ERROR: does not support in-memory (default) ledger storage." << std::endl;
    return 2;
  }

  auto storage = storage::LedgerStorage::createLedgerStorage(config.storageType, config.ledgerPrefix, config.storagePath);
  Name trackerName = dag::toStateListName(dag::globalTracker);
  auto trackerBlock = storage->getBlock(trackerName);
  dag::EdgeStateList statesTracker = dag::decodeEdgeStateList(trackerBlock);
  std::cerr << "There are " << statesTracker.value.size() << " EdgeStates: " << std::endl;
  for (const auto& entry : statesTracker.value) {
    auto stateBlock = storage->getBlock(entry);
    dag::EdgeState state = dag::decodeEdgeState(stateBlock);
    if (!isBenchmark) {
        std::cerr << "***************************************\n"
                  << state
                  << "***************************************\n";
    }
    if (state.interlocked > state.created) {
        std::cerr << "=======================================\n"
                  << state.stateName << " Interlock Latency: "
                  << ndn::time::toUnixTimestamp(state.interlocked) - ndn::time::toUnixTimestamp(state.created) << " ms\n"
                  << "=======================================\n";
    }

  }
  return 0;
}

} // namespace cledger::ledger

int
main(int argc, char* argv[])
{
  return cledger::ledger::main(argc, argv);
}
