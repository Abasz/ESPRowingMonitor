// NOLINTBEGIN(readability-magic-numbers)
#include <array>
#include <string>
#include <vector>

#include "../include/catch_amalgamated.hpp"
#include "../include/fakeit.hpp"

#include "../include/Arduino.h"
#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/callbacks/ota.callbacks.h"
#include "../../../src/utils/configuration.h"
#include "../../../src/utils/enums.h"
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("OtaRxCallbacks onWrite method should", "[ota]")
{
    Mock<IOtaUploaderService> mockOtaService;
    Mock<NimBLECharacteristic> mockOtaRxCharacteristic;

    mockArduino.Reset();
    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();
    mockNimBLECharacteristic.Reset();

    ble_gap_conn_desc gapDescriptor{
        .conn_handle = 0};

    When(Method(mockOtaRxCharacteristic, getService)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockNimBLEService, getServer)).AlwaysReturn(&mockNimBLEServer.get());
    When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(23);

    When(Method(mockOtaRxCharacteristic, getValue)).AlwaysReturn({0});
    Fake(Method(mockOtaService, onData));

    OtaRxCallbacks otaCallback(mockOtaService.get());
    mockNimBLECharacteristic.ClearInvocationHistory();

    SECTION("get MTU")
    {
        otaCallback.onWrite(&mockOtaRxCharacteristic.get(), &gapDescriptor);

        Verify(Method(mockNimBLEServer, getPeerMTU)).Once();
    }

    SECTION("call OtaService with characteristic value and MTU")
    {
        const auto expectedMtu = 256U;
        NimBLEAttValue expectedValue = {1, 2, 3, 4};
        std::vector<unsigned char> expectedVector;
        expectedVector.assign(expectedValue.begin(), expectedValue.end());

        When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(expectedMtu);
        When(Method(mockOtaRxCharacteristic, getValue)).AlwaysReturn(expectedValue);

        std::vector<unsigned char> resultValue{};
        When(Method(mockOtaService, onData)).Do([&resultValue](const NimBLEAttValue &data, unsigned short mtu)
                                                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                                { resultValue.insert(end(resultValue), data.begin(), data.begin() + data.size()); });

        otaCallback.onWrite(&mockOtaRxCharacteristic.get(), &gapDescriptor);

        Verify(Method(mockOtaService, onData)).Once();

        REQUIRE_THAT(resultValue, Catch::Matchers::Equals(expectedVector));
    }
}
// NOLINTEND(readability-magic-numbers)