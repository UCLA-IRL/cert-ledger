#ifndef CLEDGER_TESTS_SVS_CORE_IDENTITY_TIME_FIXTURE_HPP
#define CLEDGER_TESTS_SVS_CORE_IDENTITY_TIME_FIXTURE_HPP

#include "identity-management-fixture.hpp"
#include "unit-test-time-fixture.hpp"
#include "svs-core-fixture.hpp"


namespace cledger::tests {

class SVSCoreIdentityTimeFixture : public IdentityManagementTimeFixture
                                 , public SVSCoreFixture
{
};

} // namespace cledger::tests

#endif // CLEDGER_TESTS_SVS_CORE_IDENTITY_TIME_FIXTURE_HPP
