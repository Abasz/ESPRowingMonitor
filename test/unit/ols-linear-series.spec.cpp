#include "catch_amalgamated.hpp"

#include "../../src/utils/ols-linear-series.h"

using namespace std;

TEST_CASE("Ordinary Least Square Linear Regression", "[regression]")
{
    OLSLinearSeries olsReg(7);

    olsReg.push(5331447, 5331447);
    olsReg.push(5646535, 315088);
    olsReg.push(5801441, 154906);
    olsReg.push(5911468, 110027);
    olsReg.push(6000923, 89455);
    olsReg.push(6078428, 77505);
    olsReg.push(6147876, 69448);
    olsReg.push(6211006, 63130);
    olsReg.push(6269272, 58266);
    olsReg.push(6323676, 54404);
    olsReg.push(6374659, 50983);
    olsReg.push(6423024, 48365);
    olsReg.push(6469352, 46328);
    olsReg.push(6513745, 44393);
    olsReg.push(6556624, 42879);
    olsReg.push(6598265, 41641);
    olsReg.push(6638598, 40333);
    olsReg.push(6677861, 39263);
    olsReg.push(6716247, 38386);
    olsReg.push(6753594, 37347);
    olsReg.push(6790080, 36486);
    olsReg.push(6825842, 35762);

    SECTION("should correctly calculate the slope")
    {
        CHECK_THAT(olsReg.slope(), Catch::Matchers::WithinRel(-0.0257004818, 0.00001));
    }

    SECTION("should correctly calculate the goodness of fit")
    {
        CHECK_THAT(olsReg.goodnessOfFit(), Catch::Matchers::WithinRel(0.9961418613, 0.00001));
    }
}
