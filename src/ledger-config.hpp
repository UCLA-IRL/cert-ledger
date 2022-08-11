#ifndef CLEDGER_LEDGER_CONFIG_HPP
#define CLEDGER_LEDGER_CONFIG_HPP

#define CLEDGER_SYSCONFDIR "/usr/local/etc"

#include "cledger-common.hpp"

namespace cledger::ledger {

/**
 * @brief Ledger's configuration on cledger.
 *
 * The format of Ledger configuration in JSON
 * {
 *  "ledger-prefix": "",
 *  "nack-freshness-period": "", (in seconds)
 *  "record-zones":
 *  [
 *    {"record-zone-prefix": ""},
 *    {"record-zone-prefix": ""}
 *  ]
 * }
 */
class LedgerConfig
{
public:
  /**
   * @brief Load Ledger configuration from the file.
   * @throw std::runtime_error when config file cannot be correctly parsed.
   */
  void
  load(const std::string& fileName);

public:
  Name ledgerPrefix;
  ndn::time::milliseconds nackFreshnessPeriod;
  // operator should list the namespace(s) that this Ledger is responsible of.
  // Ledger won't do look up for records that are that belong to any of the record Zone.
  // no protocol side impact, purely for filtering Ledger side unnecessary record look up.
  std::vector<Name> recordZones;
  std::string schemaFile;
};

} // namespace cledger::ledger

#endif // CLEDGER_LEDGER_CONFIG_HPP
