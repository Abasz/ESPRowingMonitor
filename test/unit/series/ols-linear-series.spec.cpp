#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

#include "../../../src/utils/series/ols-linear-series.h"
#include "./regression.test-cases.spec.h"

TEST_CASE("Ordinary Least Square Linear Regression", "[regression]")
{
    OLSLinearSeries olsReg(testMaxSize);

    for (const auto &testCase : testCases)
    {
        olsReg.push(testCase[0], testCase[1]);
    }

    SECTION("should correctly calculate the slope")
    {
        const auto slopeExpected = -0.0257004818;
        CHECK_THAT(olsReg.slope(), Catch::Matchers::WithinRel(slopeExpected, 0.00001));
    }

    SECTION("should correctly calculate the goodness of fit")
    {
        const auto goodnessOfFitExpected = 0.9961418613;
        CHECK_THAT(olsReg.goodnessOfFit(), Catch::Matchers::WithinRel(goodnessOfFitExpected, 0.00001));
    }
}
