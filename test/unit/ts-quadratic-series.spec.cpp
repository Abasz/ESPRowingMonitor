// NOLINTBEGIN(readability-function-cognitive-complexity,readability-magic-numbers)

#include "catch_amalgamated.hpp"

#include "../../src/utils/ts-quadratic-series.h"
#include "regression.test-cases.spec.h"

TEST_CASE("Theil Sen Quadratic Regression", "[regression]")
{
    TSQuadraticSeries tsQuad(testMaxSize);

    for (const auto &testCase : testCases)
    {
        tsQuad.push(testCase[0] / 1e6, testCase[2]);
    }

    SECTION("firstDerivativeAtPosition should return correct values")
    {
        CHECK(tsQuad.firstDerivativeAtPosition(0) == 51.217592061688464);
        CHECK(tsQuad.firstDerivativeAtPosition(1) == 52.637568843439993);
        CHECK(tsQuad.firstDerivativeAtPosition(2) == 54.019874855437877);
        CHECK(tsQuad.firstDerivativeAtPosition(3) == 55.371304918768516);
        CHECK(tsQuad.firstDerivativeAtPosition(4) == 56.686155608478515);
        CHECK(tsQuad.firstDerivativeAtPosition(5) == 57.970693650751258);
        CHECK(tsQuad.firstDerivativeAtPosition(6) == 59.229742312368245);
        CHECK(tsQuad.firstDerivativeAtPosition(7) == 0);
        CHECK(tsQuad.firstDerivativeAtPosition(8) == 0);
    }

    SECTION("secondDerivativeAtPosition should return correct values")
    {
        const auto secondDerExpected = 35.206326872574067;
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
}
// NOLINTEND(readability-function-cognitive-complexity,readability-magic-numbers)