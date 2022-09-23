#include "ledger-module.hpp"
#include "dag/edge-state-list.hpp"
#include "util/segment/consumer.hpp"
#include "util/segment/pipeline-interests-fixed.hpp"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <iostream>

namespace cledger::ledger {

static ndn::Face face;
static ndn::ValidatorConfig validator{face};
static util::segment::Options options;
static std::vector<std::shared_ptr<util::segment::Consumer>> stateConsumers;
static std::vector<std::shared_ptr<util::segment::PipelineInterestsFixed>> statePipelines;


static void
fetchEdgeState(const Name& instancePrefix,
               const Name& stateName, bool isBenchmark)
{
  auto interestName = Name(instancePrefix).appendKeyword("internal")
                                          .append(stateName);
  Interest stateFetcher(interestName);
  stateFetcher.setCanBePrefix(true);
  stateFetcher.setMustBeFresh(true);
  face.expressInterest(stateFetcher, 
    [instancePrefix, isBenchmark] (auto&&, auto& data) {
      auto consumer = std::make_shared<util::segment::Consumer>(validator, [=] (auto& block) {
        dag::EdgeState state = dag::decodeEdgeState(block);
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
      });
      // auto pipeline = std::make_shared<util::segment::PipelineInterestsFixed>(face, options);
      auto pipeline = std::make_shared<util::segment::PipelineInterestsFixed>(face, options);
      consumer->run(data.getName().getPrefix(-1), pipeline);
      stateConsumers.push_back(consumer);
      statePipelines.push_back(pipeline);
    },
    [] (auto&&...) {},
    [] (auto&&...) {}
  );
}

static void
fetchEdgeStateList(const Name& instancePrefix, bool isBenchmark)
{
  auto interestName = Name(instancePrefix).appendKeyword("internal")
                                          .append(dag::toStateListName(dag::globalTracker));
  Interest listFetcher(interestName);
  listFetcher.setCanBePrefix(true);
  listFetcher.setMustBeFresh(true);
  face.expressInterest(listFetcher, 
    [instancePrefix, isBenchmark] (auto&&, auto& data) {
      // /<obj>/data/<version>/<segment>
      auto consumer = std::make_shared<util::segment::Consumer>(validator, [=] (auto& block) {
        dag::EdgeStateList statesTracker = dag::decodeEdgeStateList(block);
        std::cerr << "There are " << statesTracker.value.size() << " EdgeStates: " << std::endl;
        for (const auto& entry : statesTracker.value) {
          fetchEdgeState(instancePrefix, entry, isBenchmark);
        }
      });
      auto pipeline = std::make_shared<util::segment::PipelineInterestsFixed>(face, options);
      consumer->run(data.getName().getPrefix(-1), pipeline);
      stateConsumers.push_back(consumer);
      statePipelines.push_back(pipeline);
    },
    [] (auto&&...) {},
    [] (auto&&...) {});
}

static int
main(int argc, char* argv[])
{
  namespace po = boost::program_options;
  std::string instancePrefixStr = "";
  std::string validatorFilePath;
  bool isBenchmark = false;
  po::options_description description(
    "Usage: ndncledger-ledger-status [-h] -i instancePrefix -d validator [-b]\n"
    "\n"
    "Options");
  description.add_options()
    ("help,h", "produce help message")
    ("instancePrefix,i",  po::value<std::string>(&instancePrefixStr),
                         "ledger prefix (e.g., /ndn/site1/instance1)")
    ("validator,d",      po::value<std::string>(&validatorFilePath),
                          "the file path to load the ndn-cxx validator (e.g., trust-schema.conf)")
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
  if (vm.count("instancePrefix") == 0) {
    std::cerr << "ERROR: you must specify an instance prefix." << std::endl;
    return 2;
  }
  validator.load(validatorFilePath);
  fetchEdgeStateList(Name(instancePrefixStr), isBenchmark);
  face.processEvents();
  return 0;
}

} // namespace cledger::ledger

int
main(int argc, char* argv[])
{
  return cledger::ledger::main(argc, argv);
}
