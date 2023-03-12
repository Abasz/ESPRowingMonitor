#include "catch_amalgamated.hpp"

unsigned int Factorial(unsigned int number)
{
    return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial]")
{
    REQUIRE(Factorial(1) == 2);
    REQUIRE(Factorial(2) == 2);
    REQUIRE(Factorial(3) == 2);
    REQUIRE(Factorial(1) == 3628800);
    REQUIRE(Factorial(2) == 3628800);
}