// NOLINTBEGIN(readability-magic-numbers)
#include <array>
#include <span>
#include <string>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"
#include "fakeit.hpp"

#include "../include/Arduino.h"
#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/callbacks/ota.callbacks.h"
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("OtaRxCallbacks onWrite method should", "[ota]")
{
    mockNimBLEServer.Reset();
    mockNimBLEService.Reset();

    Mock<IOtaUpdaterService> mockOtaService;
    Mock<NimBLECharacteristic> mockOtaRxCharacteristic;
    Mock<NimBLEConnInfo> mockConnectionInfo;

    When(Method(mockConnectionInfo, getConnHandle)).AlwaysReturn(0);

    When(Method(mockOtaRxCharacteristic, getService)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockNimBLEService, getServer)).AlwaysReturn(&mockNimBLEServer.get());
    When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(23);

    When(Method(mockOtaRxCharacteristic, getValue)).AlwaysReturn({0});
    Fake(Method(mockOtaService, onData));

    OtaRxCallbacks otaCallback(mockOtaService.get());

    SECTION("get MTU")
    {
        otaCallback.onWrite(&mockOtaRxCharacteristic.get(), mockConnectionInfo.get());

        Verify(Method(mockNimBLEServer, getPeerMTU)).Once();
    }

    SECTION("call OtaService with characteristic value and MTU")
    {
        const auto expectedMtu = 256U;
        NimBLEAttValue expectedValue = {1, 2, 3, 4};
        std::vector<unsigned char> expectedVector;
        expectedVector.assign(expectedValue.data(), expectedValue.end());

        When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(expectedMtu);
        When(Method(mockOtaRxCharacteristic, getValue)).AlwaysReturn(expectedValue);

        std::vector<unsigned char> resultValue{};
        When(Method(mockOtaService, onData)).Do([&resultValue](const NimBLEAttValue &data, unsigned short mtu)
                                                { 
                                                    const auto temp = std::span<const unsigned char>(data.data(), data.size());
                                                    resultValue.insert(cend(resultValue), cbegin(temp), cend(temp)); });

        otaCallback.onWrite(&mockOtaRxCharacteristic.get(), mockConnectionInfo.get());

        Verify(Method(mockOtaService, onData)).Once();

        REQUIRE_THAT(resultValue, Catch::Matchers::Equals(expectedVector));
    }
}
// NOLINTEND(readability-magic-numbers)