// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)
#include <array>
#include <initializer_list>

#include "catch2/catch_test_macros.hpp"

#include "../../../src/utils/series/ts-linear-series.h"
#include "./regression.test-cases.spec.h"

TEST_CASE("Theil Sen Linear Regression", "[regression]")
{
    const auto tsLinearTestMaxSize = 7U;
    TSLinearSeries tsReg(tsLinearTestMaxSize);

    for (const auto &testCase : testCases)
    {
        tsReg.push(testCase[1] / 1e6, testCase[0] / 1e6);
    }

    SECTION("median method should correctly calculate Median")
    {
        const auto expectedMedian = -39.37665713543009;
        REQUIRE(tsReg.median() == expectedMedian);
    }

    SECTION("coefficientA method should assign the median to coefficientA")
    {
        REQUIRE(tsReg.median() == tsReg.coefficientA());
    }

    SECTION("coefficientB method should calculate coefficientB correctly")
    {
        TSLinearSeries tsRegCoeffB(tsLinearTestMaxSize);

        for (const auto &testCase : testCases)
        {
            tsRegCoeffB.push(testCase[1] / 1e6, testCase[0] / 1e6);
            REQUIRE(tsRegCoeffB.coefficientB() == testCase[3]);
        }
    }

    SECTION("size method should return number of data points")
    {
        REQUIRE(tsReg.size() == tsLinearTestMaxSize);
    }

    SECTION("reset method should clear the series")
    {
        tsReg.reset();

        REQUIRE(tsReg.size() == 0);
        REQUIRE(tsReg.median() == 0);
        REQUIRE(tsReg.coefficientA() == 0);
    }

    SECTION("yAtSeriesBegin method should return first Y value")
    {
        TSLinearSeries tsRegAccessor(10);

        tsRegAccessor.push(1.0, 100.0);
        tsRegAccessor.push(2.0, 200.0);
        tsRegAccessor.push(3.0, 300.0);

        REQUIRE(tsRegAccessor.yAtSeriesBegin() == 100.0);
    }

    SECTION("xAtSeriesBegin method should return first X value")
    {
        TSLinearSeries tsRegAccessor(10);

        tsRegAccessor.push(1.0, 100.0);
        tsRegAccessor.push(2.0, 200.0);
        tsRegAccessor.push(3.0, 300.0);

        REQUIRE(tsRegAccessor.xAtSeriesBegin() == 1.0);
    }

    SECTION("xAtSeriesEnd method should return last X value")
    {
        TSLinearSeries tsRegAccessor(10);

        tsRegAccessor.push(1.0, 100.0);
        tsRegAccessor.push(2.0, 200.0);
        tsRegAccessor.push(3.0, 300.0);

        REQUIRE(tsRegAccessor.xAtSeriesEnd() == 3.0);
    }

    SECTION("with edge cases")
    {
        SECTION("median should return 0 for empty series")
        {
            TSLinearSeries tsRegEmpty(5);
            REQUIRE(tsRegEmpty.median() == 0.0);
        }

        SECTION("coefficientA should return 0 for empty series")
        {
            TSLinearSeries tsRegEmpty(5);
            REQUIRE(tsRegEmpty.coefficientA() == 0.0);
        }

        SECTION("coefficientB should return 0 for empty or single element series")
        {
            TSLinearSeries tsRegEmpty(5);
            REQUIRE(tsRegEmpty.coefficientB() == 0.0);

            tsRegEmpty.push(1.0, 10.0);
            REQUIRE(tsRegEmpty.coefficientB() == 0.0);
        }

        SECTION("should handle rolling window when maxSeriesLength is exceeded")
        {
            const auto maxLength = 3;
            TSLinearSeries tsRegRolling(maxLength);

            tsRegRolling.push(1.0, 10.0);
            tsRegRolling.push(2.0, 20.0);
            tsRegRolling.push(3.0, 30.0);

            REQUIRE(tsRegRolling.size() == 3);
            REQUIRE(tsRegRolling.xAtSeriesBegin() == 1.0);
            REQUIRE(tsRegRolling.yAtSeriesBegin() == 10.0);

            tsRegRolling.push(4.0, 40.0);

            REQUIRE(tsRegRolling.size() == 3);
            REQUIRE(tsRegRolling.xAtSeriesBegin() == 2.0);
            REQUIRE(tsRegRolling.yAtSeriesBegin() == 20.0);
            REQUIRE(tsRegRolling.xAtSeriesEnd() == 4.0);
        }
    }
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)