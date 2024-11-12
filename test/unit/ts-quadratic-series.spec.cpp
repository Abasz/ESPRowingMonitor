// NOLINTBEGIN(readability-magic-numbers)
#include "./include/catch_amalgamated.hpp"

#include "../../src/utils/series/ts-quadratic-series.h"
#include "./regression.test-cases.spec.h"

TEST_CASE("Theil Sen Quadratic Regression", "[regression]")
{
    TSQuadraticSeries tsQuad(testMaxSize);

    for (const auto &testCase : testCases)
    {
        tsQuad.push(testCase[0] / 1e6, testCase[2]);
    }

    SECTION("firstDerivativeAtPosition should return correct values")
    {
        CHECK(tsQuad.firstDerivativeAtPosition(0) == 51.21269541835392);
        CHECK(tsQuad.firstDerivativeAtPosition(1) == 52.632672200105446);
        CHECK(tsQuad.firstDerivativeAtPosition(2) == 54.01497821210333);
        CHECK(tsQuad.firstDerivativeAtPosition(3) == 55.36640827543397);
        CHECK(tsQuad.firstDerivativeAtPosition(4) == 56.68125896514397);
        CHECK(tsQuad.firstDerivativeAtPosition(5) == 57.96579700741671);
        CHECK(tsQuad.firstDerivativeAtPosition(6) == 59.2248456690337);
        CHECK(tsQuad.firstDerivativeAtPosition(7) == 0);
        CHECK(tsQuad.firstDerivativeAtPosition(8) == 0);
    }

    SECTION("secondDerivativeAtPosition should return correct values")
    {
        const auto secondDerExpected = 35.20632687257407;
        CHECK(tsQuad.secondDerivativeAtPosition(0) == secondDerExpected);
        CHECK(tsQuad.secondDerivativeAtPosition(1) == secondDerExpected);
        CHECK(tsQuad.secondDerivativeAtPosition(2) == secondDerExpected);
        CHECK(tsQuad.secondDerivativeAtPosition(3) == secondDerExpected);
        CHECK(tsQuad.secondDerivativeAtPosition(4) == secondDerExpected);
        CHECK(tsQuad.secondDerivativeAtPosition(5) == secondDerExpected);
        CHECK(tsQuad.secondDerivativeAtPosition(6) == secondDerExpected);
        CHECK(tsQuad.secondDerivativeAtPosition(7) == 0);
        CHECK(tsQuad.secondDerivativeAtPosition(8) == 0);
    }

    SECTION("should calculate correct goodness of fit")
    {
        TSQuadraticSeries tsQuadGoodness(testMaxSize);

        for (const auto &testCase : testCases)
        {
            tsQuadGoodness.push(testCase[0] / 1e6, testCase[2]);
            REQUIRE(tsQuadGoodness.goodnessOfFit() == testCase[4]);
        }
    }
}
// NOLINTEND(readability-magic-numbers)