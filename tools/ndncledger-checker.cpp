#include "checker.hpp"

#include <boost/asio.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <chrono>
#include <deque>
#include <iostream>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/io.hpp>
#include <ndn-cxx/security/transform/base64-encode.hpp>
#include <ndn-cxx/security/transform/buffer-source.hpp>
#include <ndn-cxx/security/transform/stream-sink.hpp>

namespace cledger::checker {

static ndn::Face face;
static ndn::ValidatorConfig validator{face};
static ndn::KeyChain keyChain;
static std::shared_ptr<Checker> checker;
static ssize_t nStep = 0;

template<typename T>
T
loadFromFile(const std::string& filename)
{
  try {
    if (filename == "-") {
      return ndn::io::loadTlv<T>(std::cin, ndn::io::BASE64);
    }

    std::ifstream file(filename);
    if (!file) {
      NDN_THROW(std::runtime_error("Cannot open '" + filename + "'"));
    }
    return ndn::io::loadTlv<T>(file, ndn::io::BASE64);
  }
  catch (const ndn::io::Error& e) {
    NDN_THROW_NESTED(std::runtime_error("Cannot load '" + filename +
                                        "': malformed TLV or not in base64 format (" + e.what() + ")"));
  }
}

Name
captureKeyName(ssize_t& nStep, ndn::security::pib::Identity& identity)
{
  size_t count = 0;
  std::cerr << "***************************************\n"
            << "Step " << nStep++ << ": KEY SELECTION" << std::endl;
  for (const auto& key : identity.getKeys()) {
    std::cerr << "> Index: " << count++ << std::endl
              << ">> Key Name:";
    if (key == identity.getDefaultKey()) {
      std::cerr << "  +->* ";
    }
    else {
      std::cerr << "  +->  ";
    }
    std::cerr << key.getName() << std::endl;
  }

  std::cerr << "Please type in the key's index that you want to select:\n";
  std::string indexStr = "";
  std::string indexStrLower = "";
  size_t keyIndex;
  getline(std::cin, indexStr);

  indexStrLower = indexStr;
  boost::algorithm::to_lower(indexStrLower);
	try {
		keyIndex = std::stoul(indexStr);
	}
	catch (const std::exception&) {
		std::cerr << "Your input is not valid index. Exit" << std::endl;
		exit(1);
	}

	if (keyIndex >= count) {
		std::cerr << "Your input is not an existing index. Exit" << std::endl;
		exit(1);
	}
	else {
		auto itemIterator = identity.getKeys().begin();
		std::advance(itemIterator, keyIndex);
		auto targetKeyItem = *itemIterator;
		return targetKeyItem.getName();
	}
}

Name
captureCertName(ssize_t& nStep, ndn::security::pib::Key& key)
{
  size_t count = 0;
  std::cerr << "***************************************\n"
            << "Step " << nStep++ << ": CERTIFICATE SELECTION" << std::endl;
  for (const auto& cert : key.getCertificates()) {
    std::cerr << "> Index: " << count++ << std::endl
              << ">> Certificate Name:";
    if (cert == key.getDefaultCertificate()) {
      std::cerr << "  +->* ";
    }
    else {
      std::cerr << "  +->  ";
    }
    std::cerr << cert.getName() << std::endl;
  }

  std::cerr << "Please type in the key's index that you want to select:\n";
  std::string indexStr = "";
  std::string indexStrLower = "";
  size_t certIndex;
  getline(std::cin, indexStr);

  indexStrLower = indexStr;
  boost::algorithm::to_lower(indexStrLower);
	try {
		certIndex = std::stoul(indexStr);
	}
	catch (const std::exception&) {
		std::cerr << "Your input is not valid index. Exit" << std::endl;
		exit(1);
	}

	if (certIndex >= count) {
		std::cerr << "Your input is not an existing index. Exit" << std::endl;
		exit(1);
	}
	else {
		auto itemIterator = key.getCertificates().begin();
		std::advance(itemIterator, certIndex);
		auto targetCertItem = *itemIterator;
		return targetCertItem.getName();
	}
}

/**
 * Modified from ndn-cxx
 */
ndn::security::Certificate
getCertificateFromPib(ssize_t& nStep,
                      const ndn::security::pib::Pib& pib, const Name& name,
                      bool isIdentityName, bool isKeyName, bool isCertName)
{
  if (isIdentityName) {
    auto identity = pib.getIdentity(name);
    if (identity.getKeys().size() > 1) {
      return getCertificateFromPib(nStep, pib,
          captureKeyName(nStep, identity), false, true, false);
    }
    else {
      return getCertificateFromPib(nStep, pib,
        identity.getDefaultKey().getName(), false, true, false);    
    }
  }
  else if (isKeyName) {
    auto key = pib.getIdentity(ndn::security::extractIdentityFromKeyName(name))
                  .getKey(name);
		if (key.getCertificates().size() > 1) {
			return getCertificateFromPib(nStep, pib,
        captureCertName(nStep, key), false, false, true);
		}
    else {
      return getCertificateFromPib(nStep, pib,
        key.getDefaultCertificate().getName(), false, false, true);
    }
  }
  else if (isCertName) {
    return pib.getIdentity(ndn::security::extractIdentityFromCertName(name))
           .getKey(ndn::security::extractKeyNameFromCertName(name))
           .getCertificate(name);
  }
  // should never be called
  return pib.getIdentity(ndn::security::extractIdentityFromCertName(name))
           .getKey(ndn::security::extractKeyNameFromCertName(name))
           .getCertificate(name);
}

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
checkRecords(const Certificate& cert, const Name& ledgerName,
             ndn::security::Validator& validator, bool isPretty)
{
  checker = std::make_shared<Checker>(face, validator);
  checker->doCheck(ledgerName, cert, 
    [isPretty] (auto&&, auto& i) {
      // should be a nack data
      if (isPretty) {
        std::cerr << i << std::endl;
      }
    },
    [isPretty] (auto&&, auto& i) {
      // should be a data packet
      if (isPretty) {
        std::cerr << i << std::endl;
      }
    },
    [] (auto&&, auto& i) {
      std::cerr << "ERROR: Failed because of: " << i << std::endl;
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
    "Usage: ndncledger-checker [-h] [-p] [-b NOTBEFORE] [-l LEDGERPREFIX] [-d VALIDATOR ] [-i|-k|-f] [-n] NAME\n"
    "\n"
    "Options");
  description.add_options()
    ("help,h",           "produce help message")
    ("pretty,p",         po::bool_switch(&isPretty), "display the revocation record or nack in human readable format")
    ("identity,i",       po::bool_switch(&isIdentityName),
                         "treat the NAME argument as an identity name (e.g., /ndn/edu/ucla/alice)")
    ("key,k",            po::bool_switch(&isKeyName),
                         "treat the NAME argument as a key name (e.g., /ndn/edu/ucla/alice/ksk-123456789)")
    ("file,f",           po::bool_switch(&isFileName),
                         "treat the NAME argument as the name of a file containing a base64-encoded "
                         "certificate, '-' for stdin")
    ("name,n",           po::value<std::string>(&name),
                         "unless overridden by -i/-k/-f, the name of the certificate to be revoked "
                         "(e.g., /ndn/edu/ucla/KEY/cs/alice/ksk-1234567890/ID-CERT/%FD%FF%FF%FF%FF%FF%FF%FF)")
    ("ledger-prefix,l",   po::value<std::string>(&ledgerPrefix),
                         "ledger prefix (e.g., /example/LEDGER)")
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

  int nIsNameOptions = isIdentityName + isKeyName + isFileName;
  if (nIsNameOptions > 1) {
    std::cerr << "ERROR: at most one of '--identity', '--key', "
                 "or '--file' may be specified" << std::endl;
    return 2;
  }

  ndn::security::Certificate certificate;
  if (isFileName) {
    certificate = loadFromFile<ndn::security::Certificate>(name);
  }
  else {
    certificate = getCertificateFromPib(nStep, keyChain.getPib(), name,
                    isIdentityName, isKeyName, nIsNameOptions == 0);
  }
  std::cerr << "Checking " << certificate.getName() << "...\n";
  validator.load(validatorFilePath);
  checkRecords(certificate, Name(ledgerPrefix), validator, isPretty);
  face.processEvents();
  return 0;
}
} // namespace cledger::checker

int
main(int argc, char* argv[])
{
  return cledger::checker::main(argc, argv);
}