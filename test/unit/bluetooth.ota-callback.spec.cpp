#include <array>
#include <string>
#include <vector>

#include "./include/catch_amalgamated.hpp"
#include "./include/fakeit.hpp"

#include "./include/Arduino.h"
#include "./include/NimBLEDevice.h"

#include "../../src/peripherals/bluetooth/bluetooth.controller.h"
#include "../../src/peripherals/sd-card/sd-card.service.interface.h"
#include "../../src/utils/EEPROM.service.interface.h"
#include "../../src/utils/configuration.h"
#include "../../src/utils/enums.h"
#include "../../src/utils/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("BluetoothController OtaRxCallbacks onWrite method should", "[ota]")
{
    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISdCardService> mockSdCardService;
    Mock<IOtaUploaderService> mockOtaService;
    Mock<NimBLECharacteristic> mockOtaRxCharacteristic;

    mockArduino.Reset();
    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();
    mockNimBLECharacteristic.Reset();

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockNimBLEService.get());
    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short))).AlwaysReturn(&mockNimBLEService.get());
    Fake(Method(mockNimBLEServer, createServer));
    Fake(Method(mockNimBLEServer, init));
    Fake(Method(mockNimBLEServer, setPower));
    Fake(Method(mockNimBLEServer, start));

    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockNimBLECharacteristic.get());
    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))).AlwaysReturn(&mockNimBLECharacteristic.get());
    When(Method(mockNimBLEService, getServer)).AlwaysReturn(&mockNimBLEServer.get());
    Fake(Method(mockNimBLEService, start));

    When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(0);
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 1U>)));
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const unsigned short)));
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::string)));
    Fake(Method(mockNimBLECharacteristic, notify));

    Fake(Method(mockNimBLEAdvertising, start));
    Fake(Method(mockNimBLEAdvertising, setAppearance));
    Fake(Method(mockNimBLEAdvertising, addServiceUUID));

    When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);
    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(true);
    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(true);
    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(ArduinoLogLevel::LogLevelSilent);

    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(true);

    Fake(Method(mockOtaService, begin));

    // Test specific mocks
    ble_gap_conn_desc gapDescriptor = {0};

    When(
        OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
            .Using(CommonBleFlags::otaRxUuid, Any()))
        .AlwaysReturn(&mockOtaRxCharacteristic.get());
    When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(23);

    When(Method(mockOtaRxCharacteristic, getService)).AlwaysReturn(&mockNimBLEService.get());

    When(Method(mockOtaRxCharacteristic, getValue)).AlwaysReturn({0});

    Fake(Method(mockOtaService, onData));

    BluetoothController bluetoothController(mockEEPROMService.get(), mockSdCardService.get(), mockOtaService.get());
    bluetoothController.setup();
    mockNimBLECharacteristic.ClearInvocationHistory();
    NimBLECharacteristicCallbacks *otaCallback = std::move(mockOtaRxCharacteristic.get().callbacks);

    SECTION("get MTU")
    {
        otaCallback->onWrite(&mockOtaRxCharacteristic.get(), &gapDescriptor);

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
                                                { resultValue.assign(data.begin(), data.end()); });

        otaCallback->onWrite(&mockOtaRxCharacteristic.get(), &gapDescriptor);

        Verify(Method(mockOtaService, onData)).Once();

        REQUIRE_THAT(resultValue, Catch::Matchers::Equals(expectedVector));
    }
}