#include "append/client.hpp"
#include "util.hpp"

#include <iostream>

#include <boost/asio.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/io.hpp>

namespace cledger::client {

static ndn::Face face;
static ndn::ValidatorConfig validator{face};
static ndn::KeyChain keyChain;
static std::shared_ptr<append::Client> client;
static ssize_t nStep=0;

using registerContinuation = std::function<void(void)>; // continuation after registering prefix

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
registerPrefix(const Certificate& caCert, const registerContinuation continuation)
{
  Name caPrefix = ndn::security::extractIdentityFromCertName(caCert.getName());
  face.registerPrefix(
    caPrefix,
    [caCert, continuation] (const Name& name) {
      // provide revoker's own certificate
      // notice: this only register FIB to Face, not NFD.
      face.setInterestFilter(caCert.getName(),
			  [caCert] (auto&&, const auto& i) {
					 face.put(caCert);
				}
			);
			continuation();
    },
    [caPrefix] (auto&&, const auto& reason) { 
			std::cerr << "Prefix registeration of " << caPrefix << "failed.\n"
			          << "Reason: " << reason << "\nQuit\n";
		}
  );
}

static void
submitRecord(const Name& caPrefix, const Name& ledgerName, const std::shared_ptr<Data>& data)
{
	std::cerr << "Submitting record to "<< ledgerName << "...\n";
	// use record KeyLocator as prefix
	client = std::make_shared<append::Client>(caPrefix, face, keyChain, validator);

	std::string errorMsg = "ERROR: Ledger cannot log the submitted record because of ";
	client->appendData(Name(ledgerName).append("append"), {*data},
		[errorMsg] (auto&&, auto& ack) {
			using aa = appendtlv::AppendStatus;
			Block content = ack.getContent();
			content.parse();
			for (auto elem : content.elements()) {
				uint64_t status = readNonNegativeInteger(elem);
				switch (static_cast<aa>(status)) {
					case aa::SUCCESS:
						std::cerr << "Submission Success!\n";
						break;
					case aa::FAILURE_NACK:
						std::cerr << errorMsg
											<< "Ledger Interest NACK\n";
						break;
					case aa::FAILURE_STORAGE:
						std::cerr << errorMsg
											<< "Internal storage error "
											<< "(ledger may have logged the same record)\n";
						break;
					case aa::FAILURE_TIMEOUT:
						std::cerr << errorMsg 
											<< "Ledger Interest timeout\n";
						break;
					case aa::FAILURE_VALIDATION_APP:
						std::cerr << errorMsg 
											<< "Submitted Record does not conform to trust schema\n";
						break;
					case aa::FAILURE_VALIDATION_PROTO:
						std::cerr << errorMsg
											<< "Submission Protocol Data does not conform to trust schema\n";
						break;
					default:
						std::cerr << errorMsg
											<< "Unknown errors\n";
						break;
				}
			}
			std::cerr << "Quit.\n";
			exit(1);
		},
		[errorMsg] (auto&&, auto& error) {
			std::cerr << errorMsg << error.getInfo()
								<< "\nQuit.\n";
			exit(1);
		}
	);
}

static int 
main(int argc, char* argv[])
{
  boost::asio::signal_set terminateSignals(face.getIoService());
  terminateSignals.add(SIGINT);
  terminateSignals.add(SIGTERM);
  terminateSignals.async_wait(handleSignal);

  std::string validatorFilePath;
  std::string certFilePath;
  std::string certName;
  std::string certIdentity;

  namespace po = boost::program_options;
  std::string name;
  bool isIdentityName = false;
  bool isKeyName = false;
  bool isFileName = false;
  bool isPretty = false;
  std::string ledgerPrefix;

  po::options_description description(
    "Usage: cledger-ca [-h] [-p] [-l LEDGERPREFIX] [-i|-k|-f] [-n] NAME\n"
    "\n"
    "Options");
  description.add_options()
  ("help,h",           "produce help message")
  ("pretty,p",         po::bool_switch(&isPretty), "display the revocation record in human readable format")
  ("ledger-prefix,l",  po::value<std::string>(&ledgerPrefix),
                         "ledger prefix (e.g., /example/LEDGER)")
  ("identity,i",       po::bool_switch(&isIdentityName),
                         "treat the NAME argument as an identity name (e.g., /ndn/edu/ucla/alice)")
  ("key,k",            po::bool_switch(&isKeyName),
                       "treat the NAME argument as a key name (e.g., /ndn/edu/ucla/alice/ksk-123456789)")
  ("file,f",           po::bool_switch(&isFileName),
                       "treat the NAME argument as the name of a file containing a base64-encoded "
                       "certificate, '-' for stdin")
  ;
  po::positional_options_description p;
  p.add("name", 1);

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(description).positional(p).run(), vm);
    po::notify(vm);
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n\n"
              << description << std::endl;
    return 2;
  }

  if (vm.count("help") > 0) {
    std::cout << description << std::endl;
    return 0;
  }

  if (vm.count("name") == 0) {
    std::cerr << "ERROR: you must specify a name" << std::endl;
    return 2;
  }

  int nIsNameOptions = isIdentityName + isKeyName + isFileName;
  if (nIsNameOptions > 1) {
    std::cerr << "ERROR: at most one of '--identity', '--key', "
                 "or '--file' may be specified" << std::endl;
    return 2;
  }

  ndn::security::Certificate certificate;
  if (isFileName) {
    certificate = util::loadFromFile<ndn::security::Certificate>(name);
  }
  else {
    certificate = util::getCertificateFromPib(nStep, keyChain.getPib(), name,
                    isIdentityName, isKeyName, nIsNameOptions == 0);
  }

  auto certNotBefore = certificate.getValidityPeriod().getPeriod().first;
	auto certNotAfter = certificate.getValidityPeriod().getPeriod().second;
	if (certNotBefore > notBefore || certNotAfter < notBefore)
	{
		std::cerr << "ERROR: the notBefore is outside of certificate ValidityPeriod: "
							<< ndn::time::toString(certNotBefore) << "-"
							<< ndn::time::toString(certNotAfter) << std::endl;
		return 2;		
	}

  std::shared_ptr<Data> recordData(certificate);

  if (isPretty) {
    std::cout << record::Record(*recordData) << std::endl;
  }
	else {
    using namespace ndn::security::transform;
    bufferSource(recordData->wireEncode()) >> base64Encode(true) >> streamSink(std::cout);
	}

  // submit the issued certificate to Ledger
	if (!ledgerPrefix.empty()) {
		Name ledgerName(ledgerPrefix);
		// use record KeyLocator as prefix
		Name keyLocator = recordData->getKeyLocator().value().getName();
		Name caPrefix = ndn::security::extractIdentityFromCertName(keyLocator);
		validator.load(validatorFilePath);

    Certificate caCert = util::getCertificateFromPib(nStep, keyChain.getPib(), keyLocator,
      false, false, true);
		registerPrefix(caCert,
	    [caPrefix, ledgerName, recordData]() {
		    submitRecord(caPrefix, ledgerName, recordData);
		  }
		);
	}
  face.processEvents();
  return 0;
}

} // namespace cledger::ca

int main(int argc, char* argv[])
{
  return cledger::ca::main(argc, argv);
}