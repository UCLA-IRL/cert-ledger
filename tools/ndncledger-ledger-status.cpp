#include "ledger-module.hpp"
#include "dag/edge-state-list.hpp"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <iostream>

namespace cledger::ledger {

static ndn::Face face;

static void
fetchEdgeState(const Name& ledgerPrefix, const Name& instancePrefix,
               const Name& stateName, bool isBenchmark)
{
  auto interestName = Name(instancePrefix).appendKeyword("internal")
                                          .append(stateName);
  Interest listFetcher(interestName);
  listFetcher.setCanBePrefix(true);
  listFetcher.setForwardingHint({ledgerPrefix});
  listFetcher.setMustBeFresh(true);
  face.expressInterest(listFetcher, 
    [isBenchmark] (auto&&, auto& data) {
      Block contentBlock = data.getContent();
      Block stateBlock = contentBlock.blockFromValue();
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
    },
    [] (auto&&...) {},
    [] (auto&&...) {});
}

static void
fetchEdgeStateList(const Name& ledgerPrefix, const Name& instancePrefix, bool isBenchmark)
{
  auto interestName = Name(instancePrefix).appendKeyword("internal")
                                          .append(dag::toStateListName(dag::globalTracker));
  Interest listFetcher(interestName);
  listFetcher.setCanBePrefix(true);
  listFetcher.setForwardingHint({ledgerPrefix});
  listFetcher.setMustBeFresh(true);
  face.expressInterest(listFetcher, 
    [ledgerPrefix, instancePrefix, isBenchmark] (auto&&, auto& data) {
      Block contentBlock = data.getContent();
      Block trackerBlock = contentBlock.blockFromValue();
      dag::EdgeStateList statesTracker = dag::decodeEdgeStateList(trackerBlock);
      std::cerr << "There are " << statesTracker.value.size() << " EdgeStates: " << std::endl;
      for (const auto& entry : statesTracker.value) {
        fetchEdgeState(ledgerPrefix, instancePrefix, entry, isBenchmark);
      }
    },
    [] (auto&&...) {},
    [] (auto&&...) {});
}

static int
main(int argc, char* argv[])
{
  namespace po = boost::program_options;
  std::string ledgerPrefixStr = "";
  std::string instancePrefixStr = "";
  bool isBenchmark = false;
  po::options_description description(
    "Usage: ndncledger-ledger-status [-h] -l ledgerPrefix -i instancePrefix [-b]\n"
    "\n"
    "Options");
  description.add_options()
    ("help,h", "produce help message")
    ("ledgerPrefix,l",  po::value<std::string>(&ledgerPrefixStr),
                         "ledger prefix (e.g., /ndn/site1/LEDGER)")
    ("instancePrefix,i",  po::value<std::string>(&instancePrefixStr),
                         "ledger prefix (e.g., /ndn/site1/instance1)")
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
  if (vm.count("ledgerPrefix") == 0) {
    std::cerr << "ERROR: you must specify a ledger prefix." << std::endl;
    return 2;
  }
  if (vm.count("instancePrefix") == 0) {
    std::cerr << "ERROR: you must specify an instance prefix." << std::endl;
    return 2;
  }
  fetchEdgeStateList(Name(ledgerPrefixStr), Name(instancePrefixStr), isBenchmark);
  face.processEvents();
  return 0;
}

} // namespace cledger::ledger

int
main(int argc, char* argv[])
{
  return cledger::ledger::main(argc, argv);
}
