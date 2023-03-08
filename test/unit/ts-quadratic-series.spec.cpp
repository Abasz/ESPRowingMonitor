#include "catch_amalgamated.hpp"

#include "../../src/utils/ts-quadratic-series.h"

using namespace std;

TEST_CASE("Theil Sen Quadratic Regression", "[regression]")
{
    TSQuadraticSeries tsQuad(7);

    tsQuad.push(5.331447, 2.09439510239319);
    tsQuad.push(5.646535, 4.18879020478638);
    tsQuad.push(5.801441, 6.28318530717957);
    tsQuad.push(5.911468, 8.37758040957276);
    tsQuad.push(6.000923, 10.4719755119659);
    tsQuad.push(6.078428, 12.5663706143591);
    tsQuad.push(6.147876, 14.6607657167523);
    tsQuad.push(6.211006, 16.7551608191455);
    tsQuad.push(6.269272, 18.8495559215387);
    tsQuad.push(6.323676, 20.9439510239319);
    tsQuad.push(6.374659, 23.0383461263251);
    tsQuad.push(6.423024, 25.1327412287183);
    tsQuad.push(6.469352, 27.2271363311115);
    tsQuad.push(6.513745, 29.3215314335047);
    tsQuad.push(6.556624, 31.4159265358978);
    tsQuad.push(6.598265, 33.510321638291);
    tsQuad.push(6.638598, 35.6047167406842);
    tsQuad.push(6.677861, 37.6991118430774);
    tsQuad.push(6.716247, 39.7935069454706);
    tsQuad.push(6.753594, 41.8879020478638);
    tsQuad.push(6.79008, 43.982297150257);
    tsQuad.push(6.825842, 46.0766922526502);

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
        CHECK(tsQuad.secondDerivativeAtPosition(0) == 36.30125477962745);
        CHECK(tsQuad.secondDerivativeAtPosition(1) == 36.30125477962745);
        CHECK(tsQuad.secondDerivativeAtPosition(2) == 36.30125477962745);
        CHECK(tsQuad.secondDerivativeAtPosition(3) == 36.30125477962745);
        CHECK(tsQuad.secondDerivativeAtPosition(4) == 36.30125477962745);
        CHECK(tsQuad.secondDerivativeAtPosition(5) == 36.30125477962745);
        CHECK(tsQuad.secondDerivativeAtPosition(6) == 36.30125477962745);
        CHECK(tsQuad.secondDerivativeAtPosition(7) == 0);
        CHECK(tsQuad.secondDerivativeAtPosition(8) == 0);
    }
}
