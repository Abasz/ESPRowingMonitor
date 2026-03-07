// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)
#include <array>
#include <initializer_list>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
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

    SECTION("slope method should correctly calculate the slope")
    {
        const auto slopeExpected = -0.0257004818;
        CHECK_THAT(olsReg.slope(), Catch::Matchers::WithinRel(slopeExpected, 0.00001));
    }

    SECTION("goodnessOfFit method should correctly calculate the goodness of fit")
    {
        const auto goodnessOfFitExpected = 0.9961418613;
        CHECK_THAT(olsReg.goodnessOfFit(), Catch::Matchers::WithinRel(goodnessOfFitExpected, 0.00001));
    }

    SECTION("intercept method should correctly calculate the intercept")
    {
        const auto interceptExpected = 211'021.88;
        CHECK_THAT(olsReg.intercept(), Catch::Matchers::WithinRel(interceptExpected, 0.00001));
    }

    SECTION("yAtSeriesBegin method should return first Y value in rolling window")
    {
        // With maxSeriesLength=7 and 22 test cases, first Y is from 16th test case (index 15)
        const auto expectedFirstY = 41'641.0;
        REQUIRE(olsReg.yAtSeriesBegin() == expectedFirstY);
    }

    SECTION("size method should return number of data points")
    {
        REQUIRE(olsReg.size() == 7);
    }

    SECTION("reset method should clear all internal series")
    {
        olsReg.reset();

        REQUIRE(olsReg.size() == 0);
        REQUIRE(olsReg.slope() == 0.0);
        REQUIRE(olsReg.intercept() == 0.0);
        REQUIRE(olsReg.goodnessOfFit() == 0.0);
    }

    SECTION("with edge cases")
    {
        SECTION("slope should return 0 when size is less than 2")
        {
            OLSLinearSeries olsRegEmpty(5);

            REQUIRE(olsRegEmpty.slope() == 0.0);

            olsRegEmpty.push(1.0, 2.0);
            REQUIRE(olsRegEmpty.slope() == 0.0);
        }

        SECTION("intercept should return 0 when size is less than 2")
        {
            OLSLinearSeries olsRegEmpty(5);

            REQUIRE(olsRegEmpty.intercept() == 0.0);

            olsRegEmpty.push(1.0, 2.0);
            REQUIRE(olsRegEmpty.intercept() == 0.0);
        }

        SECTION("goodnessOfFit should return 0 when size is less than 2")
        {
            OLSLinearSeries olsRegEmpty(5);

            REQUIRE(olsRegEmpty.goodnessOfFit() == 0.0);

            olsRegEmpty.push(1.0, 2.0);
            REQUIRE(olsRegEmpty.goodnessOfFit() == 0.0);
        }

        SECTION("should handle rolling window when maxSeriesLength is exceeded")
        {
            const auto maxLength = 3;
            OLSLinearSeries olsRegRolling(maxLength);

            olsRegRolling.push(1.0, 1.0);
            olsRegRolling.push(2.0, 2.0);
            olsRegRolling.push(3.0, 3.0);

            REQUIRE(olsRegRolling.size() == 3);
            REQUIRE(olsRegRolling.yAtSeriesBegin() == 1.0);

            olsRegRolling.push(4.0, 4.0);

            REQUIRE(olsRegRolling.size() == 3);
            REQUIRE(olsRegRolling.yAtSeriesBegin() == 2.0);
        }
    }
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)