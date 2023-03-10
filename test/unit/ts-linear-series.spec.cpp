#include "catch_amalgamated.hpp"

#include "../../src/utils/ts-linear-series.h"
#include "regression.test-cases.spec.h"

TEST_CASE("Theil Sen Linear Regression", "[regression]")
{
    const auto testMaxSize = 7U;
    TSLinearSeries tsReg(testMaxSize);

    for (auto &testCase : testCases)
    {
        tsReg.push(testCase[1] / 1e6, testCase[0] / 1e6);
    }

    SECTION("should correctly calculate Median")
    {
        const auto expectedMedian = -39.37665713543009;
        REQUIRE(tsReg.median() == expectedMedian);
    }
    SECTION("should assign the median to coefficientA")
    {
        REQUIRE(tsReg.median() == tsReg.coefficientA());
    }

    SECTION("should be empty after reset")
    {
        tsReg.reset();
        REQUIRE(tsReg.median() == 0);
    }
}