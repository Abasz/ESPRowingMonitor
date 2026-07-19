#include <array>
#include <cstring>
#include <functional>
#include <string>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "./include/Arduino.h"
#include "./include/esp_err.h"

#include "./include/globals.h"

using namespace fakeit;
// NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
enum esp_mac_type_t : unsigned char;

TEST_CASE("generateSerial", "[globals]")
{
    mockArduino.Reset();

    When(Method(mockArduino, esp_read_mac))
        .AlwaysDo([](unsigned char *mac, esp_mac_type_t) -> int
                  {
                      constexpr std::array<unsigned char, 6> defaultMac = {0x01, 0x11, 0x10, 0x09, 0x08, 0x07};
                      std::memcpy(mac, defaultMac.data(), defaultMac.size());

                      return ESP_OK; });

    SECTION("should return the last 3 section of the MAC address")
    {
        const auto result = generateSerial();

        REQUIRE(result == "090807");
    }

    SECTION("should return zeros when esp_read_mac failes")
    {
        When(Method(mockArduino, esp_read_mac)).AlwaysReturn(ESP_FAIL);

        const auto result = generateSerial();

        REQUIRE(result == "000000");
    }
}
