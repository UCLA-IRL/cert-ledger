#ifndef CLEDGER_TESTS_SVS_CORE_IDENTITY_TIME_FIXTURE_HPP
#define CLEDGER_TESTS_SVS_CORE_IDENTITY_TIME_FIXTURE_HPP

#include "svs-core-fixture.hpp"
#include "identity-management-time-fixture.hpp"

namespace cledger::tests {

class SVSCoreIdentityTimeFixture : public SVSCoreFixture
                                 , public IdentityManagementTimeFixture
{
};

} // namespace cledger::tests

#endif // CLEDGER_TESTS_SVS_CORE_IDENTITY_TIME_FIXTURE_HPP
