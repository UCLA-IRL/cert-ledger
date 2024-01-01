#include "append/client.hpp"
#include "util/io.hpp"

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
  face.getIoContext().stop();
  exit(1);
}

static void 
registerPrefix(const Name& clientName, const registerContinuation continuation)
{
  face.registerPrefix(
    clientName,
    [clientName, continuation] (const Name& name) {
      // provide client's own certificate
      // notice: this only register FIB to Face, not NFD.

      auto clientCert = keyChain.getPib().getIdentity(clientName)
                                         .getDefaultKey().getDefaultCertificate();
      face.setInterestFilter(clientCert.getName(),
			  [clientCert] (auto&&, const auto& i) {
					 face.put(clientCert);
				}
			);
			continuation();
    },
    [] (auto& name, const auto& reason) { 
			std::cerr << "Prefix registeration of " << name << "failed.\n"
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
      face.getIoContext().stop();
		},
		[errorMsg] (auto&&, auto& error) {
			std::cerr << errorMsg << error.getInfo()
								<< "\nQuit.\n";
      face.getIoContext().stop();
		}
	);
}

static int 
main(int argc, char* argv[])
{
  boost::asio::signal_set terminateSignals(face.getIoContext());
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
  std::string ledgerPrefix;
  std::string clientPrefix;

  po::options_description description(
    "Usage: cledger-ca [-h] [-l LEDGERPREFIX] [-c CLIENTPREFIX] [-i|-k|-f] [-n] NAME\n"
    "\n"
    "Options");
  description.add_options()
  ("help,h",           "produce help message")
  ("ledger-prefix,l",  po::value<std::string>(&ledgerPrefix),
                         "ledger prefix (e.g., /example/LEDGER)")
  ("client-prefix,c",  po::value<std::string>(&clientPrefix),
                         "client prefix (e.g., /example/client)")
  ("identity,i",       po::bool_switch(&isIdentityName),
                        "treat the NAME argument as an identity name (e.g., /ndn/edu/ucla/cs/tianyuan)")
  ("key,k",            po::bool_switch(&isKeyName),
                        "treat the NAME argument as a key name (e.g., /ndn/edu/ucla/cs/tianyuan/KEY/%25%F3Jki%09%9E%9D)")
  ("file,f",           po::bool_switch(&isFileName),
                        "treat the NAME argument as the name of a file containing a base64-encoded "
                        "certificate, '-' for stdin")
  ("name,n",           po::value<std::string>(&name),
                        "unless overridden by -i/-k/-f, the name of the certificate to be revoked "
                        "(e.g., /ndn/edu/ucla/cs/tianyuan/KEY/%25%F3Jki%09%9E%9D/NDNCERT/v=1662276808210)")
  ("validator,d",      po::value<std::string>(&validatorFilePath),
                        "the file path to load the ndn-cxx validator (e.g., trust-schema.conf)")
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

  if (vm.count("ledger-prefix") == 0) {
    std::cerr << "ERROR: you must specify a ledger prefix" << std::endl;
    return 2;
  }

  if (vm.count("client-prefix") == 0) {
    std::cerr << "ERROR: you must specify a client prefix" << std::endl;
    return 2;
  }

  if (vm.count("validator") == 0) {
    std::cerr << "ERROR: you must specify a validator" << std::endl;
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

  auto recordData = std::make_shared<Data>(certificate);
  // submit the issued certificate to Ledger
	if (!ledgerPrefix.empty()) {
		// use record KeyLocator as prefix
		Name keyLocator = recordData->getKeyLocator().value().getName();
		Name caPrefix = ndn::security::extractIdentityFromCertName(keyLocator);
		validator.load(validatorFilePath);
		registerPrefix(Name(clientPrefix),
	    [clientPrefix, ledgerPrefix, recordData]() {
		    submitRecord(Name(clientPrefix), Name(ledgerPrefix), recordData);
		  }
		);
	}
  face.processEvents();
  return 0;
}

} // namespace cledger::ca

int main(int argc, char* argv[])
{
  return cledger::client::main(argc, argv);
}