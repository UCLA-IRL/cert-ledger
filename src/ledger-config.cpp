#include "ledger-config.hpp"

#include <ndn-cxx/util/io.hpp>

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace cledger::ledger {

const std::string CONFIG_LEDGER_PREFIX = "ledger-prefix";
const std::string CONFIG_INSTANCE_SUFFIX = "instance-suffix";
const std::string CONFIG_NACK_FRESHNESS_PERIOD = "nack-freshness-period";
const std::string CONFIG_RECORD_ZONES = "record-zones";
const std::string CONFIG_STORAGE = "storage";
const std::string CONFIG_STORAGE_TYPE = "storage-type";
const std::string CONFIG_STORAGE_PATH = "storage-path";
const std::string CONFIG_INTERLOCK_POLICY = "interlock-policy";
const std::string CONFIG_INTERLOCK_POLICY_TYPE = "policy-type";
const std::string CONFIG_INTERLOCK_POLICY_THRESHOLD = "policy-threshold";
const std::string CONFIG_TRUST_SCHEMA = "trust-schema";

const std::string CONFIG_SYNC = "sync";
const std::string CONFIG_SYNC_INTEREST_SIGNING = "interest-signing";
const std::string CONFIG_SYNC_DATA_SIGNING = "data-signing";

void
LedgerConfig::load(const std::string& fileName)
{
  JsonSection configJson;
  try {
    boost::property_tree::read_json(fileName, configJson);
  }
  catch (const std::exception& error) {
    NDN_THROW(std::runtime_error("Failed to parse configuration file " + fileName + ", " + error.what()));
  }
  if (configJson.begin() == configJson.end()) {
    NDN_THROW(std::runtime_error("No JSON configuration found in file: " + fileName));
  }

  // Ledger prefix
  ledgerPrefix = Name(configJson.get(CONFIG_LEDGER_PREFIX, ""));
  if (ledgerPrefix.empty()) {
    NDN_THROW(std::runtime_error("Cannot parse ledger-prefix from the config file"));
  }
  // Instance suffix
  instanceSuffix = Name(configJson.get(CONFIG_INSTANCE_SUFFIX, ""));
  if (instanceSuffix.empty()) {
    NDN_THROW(std::runtime_error("Cannot parse instance-suffix from the config file"));
  }
  // Nack Freshness Period
  nackFreshnessPeriod = time::seconds(configJson.get(CONFIG_NACK_FRESHNESS_PERIOD, 86400));
  // Record Zones
  recordZones.clear();
  auto recordZonePrefixJson = configJson.get_child_optional(CONFIG_RECORD_ZONES);
  if (recordZonePrefixJson) {
    for (const auto& item : *recordZonePrefixJson) {
      recordZones.push_back(Name(item.second.data()));
    }
  }
  else {
    NDN_THROW(std::runtime_error("No recordZone configured."));
  }
  // Storage
  auto storageConfig = configJson.get_child_optional(CONFIG_STORAGE);
  if (storageConfig) {
    for (const auto& item : *storageConfig) {
      if (item.first == CONFIG_STORAGE_TYPE) {
        storageType = item.second.data();
      }
      else if (item.first == CONFIG_STORAGE_PATH) {
        storagePath = item.second.data();
      }
      else {
        NDN_THROW(std::runtime_error("Unrecognized keyword " + item.first));
      }
    }
  }
  else {
    storageType = "storage-memory";
    storagePath = "";
  }


  // Interlock policy
  auto interlockPolicy = configJson.get_child_optional(CONFIG_INTERLOCK_POLICY);
  if (interlockPolicy) {
    for (const auto& item : *interlockPolicy) {
      if (item.first == CONFIG_INTERLOCK_POLICY_TYPE) {
        if (!policyType.empty()) {
          NDN_THROW(std::runtime_error("There is already a policy type " + policyType));
        }
        policyType = item.second.data();
      }
      else if (item.first == CONFIG_INTERLOCK_POLICY_THRESHOLD) {
        if (policyThreshold > uint32_t(-1)) {
          NDN_THROW(std::runtime_error("There is already a policy threshold " + std::to_string(policyThreshold)));
        }
        policyThreshold = std::stoul(item.second.data());
      }
      else {
        NDN_THROW(std::runtime_error("Unrecognized keyword"));
      }
    }
  }
  else {
    NDN_THROW(std::runtime_error("No interlock policy configured."));
  }
  // Trust schema
  schemaFile = configJson.get(CONFIG_TRUST_SCHEMA, "");
  if (schemaFile.empty()) {
    NDN_THROW(std::runtime_error("Cannot parse trust schema from the config file"));
  }
  // sync
  auto syncConfig = configJson.get_child_optional(CONFIG_SYNC);
  if (syncConfig) {
    for (const auto& item : *syncConfig) {
      if (item.first == CONFIG_SYNC_INTEREST_SIGNING) {
        interestSigner = ndn::security::SigningInfo(item.second.data());
      }
      else if (item.first == CONFIG_SYNC_DATA_SIGNING) {
        dataSigner = ndn::security::SigningInfo(item.second.data());
      }
      else {
        NDN_THROW(std::runtime_error("Unrecognized keyword " + item.first));
      }
    }
  }
  else {
    NDN_THROW(std::runtime_error("Cannot parse sync config from the config file"));
  }
}

} // namespace ndnrevoke::ct
