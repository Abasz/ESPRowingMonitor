#include "catch_amalgamated.hpp"

#include "../../src/utils/ts-linear-series.h"

using namespace std;

TEST_CASE("Theil Sen Linear Regression", "[regression]")
{
    TSLinearSeries tsReg(7);

    tsReg.push(5.331447, 5.331447);
    tsReg.push(0.315088, 5.646535);
    tsReg.push(0.154906, 5.801441);
    tsReg.push(0.110027, 5.911468);
    tsReg.push(0.089455, 6.000923);
    tsReg.push(0.077505, 6.078428);
    tsReg.push(0.069448, 6.147876);
    tsReg.push(0.06313, 6.211006);
    tsReg.push(0.058266, 6.269272);
    tsReg.push(0.054404, 6.323676);
    tsReg.push(0.050983, 6.374659);
    tsReg.push(0.048365, 6.423024);
    tsReg.push(0.046328, 6.469352);
    tsReg.push(0.044393, 6.513745);
    tsReg.push(0.042879, 6.556624);
    tsReg.push(0.041641, 6.598265);
    tsReg.push(0.040333, 6.638598);
    tsReg.push(0.039263, 6.677861);
    tsReg.push(0.038386, 6.716247);
    tsReg.push(0.037347, 6.753594);
    tsReg.push(0.036486, 6.79008);
    tsReg.push(0.035762, 6.825842);

    SECTION("should correctly calculate Median")
    {
        REQUIRE(tsReg.median() == -39.37665713543009);
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