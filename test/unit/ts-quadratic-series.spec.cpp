// NOLINTBEGIN(readability-function-cognitive-complexity,readability-magic-numbers)

#include "catch_amalgamated.hpp"

#include "../../src/utils/ts-quadratic-series.h"
#include "regression.test-cases.spec.h"

TEST_CASE("Theil Sen Quadratic Regression", "[regression]")
{
    TSQuadraticSeries tsQuad(testMaxSize);

    for (auto &testCase : testCases)
    {
        tsQuad.push(testCase[0] / 1e6, testCase[2]);
    }

    SECTION("firstDerivativeAtPosition should return correct values")
    {
        CHECK(tsQuad.firstDerivativeAtPosition(0) == 51.11029639652057);
        CHECK(tsQuad.firstDerivativeAtPosition(1) == 52.57443490554732);
        CHECK(tsQuad.firstDerivativeAtPosition(2) == 53.999731071959815);
        CHECK(tsQuad.firstDerivativeAtPosition(3) == 55.39319103793059);
        CHECK(tsQuad.firstDerivativeAtPosition(4) == 56.74893400018533);
        CHECK(tsQuad.firstDerivativeAtPosition(5) == 58.073421582074815);
        CHECK(tsQuad.firstDerivativeAtPosition(6) == 59.371627055503865);
        CHECK(tsQuad.firstDerivativeAtPosition(7) == 0);
        CHECK(tsQuad.firstDerivativeAtPosition(8) == 0);
    }

    SECTION("secondDerivativeAtPosition should return correct values")
    {
        const auto secondDerExpected = 36.30125477962745;
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