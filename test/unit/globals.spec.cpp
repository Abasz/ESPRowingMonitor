#include "catch2/catch_test_macros.hpp"

#include "./include/globals.h"

TEST_CASE("generateSerial", "[globals]")
{
    SECTION("should return the last 3 section of the MAC address")
    {
        const auto result = generateSerial();

        REQUIRE(result == "090807");
    }
}