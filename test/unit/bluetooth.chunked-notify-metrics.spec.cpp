// NOLINTBEGIN(readability-magic-numbers)
#include <array>
#include <string>

#include "./include/catch_amalgamated.hpp"
#include "./include/fakeit.hpp"

#include "./include/Arduino.h"
#include "./include/NimBLEDevice.h"

#include "../../src/peripherals/bluetooth.service.h"
#include "../../src/peripherals/sd-card.service.interface.h"
#include "../../src/utils/EEPROM.service.interface.h"
#include "../../src/utils/configuration.h"
#include "../../src/utils/enums.h"
#include "../../src/utils/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("BluetoothServer ChunkedNotifyMetricCallbacks onSubscribed method", "[callbacks]")
{
    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISdCardService> mockSdCardService;
    Mock<IOtaUploaderService> mockOtaService;
    Mock<NimBLECharacteristic> mockDeltaTimesCharacteristic;

    mockArduino.Reset();
    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();
    mockNimBLECharacteristic.Reset();

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockNimBLEService.get());
    Fake(Method(mockNimBLEServer, createServer));
    Fake(Method(mockNimBLEServer, init));
    Fake(Method(mockNimBLEServer, setPower));
    Fake(Method(mockNimBLEServer, start));

    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockNimBLECharacteristic.get());
    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))).AlwaysReturn(&mockNimBLECharacteristic.get());
    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short))).AlwaysReturn(&mockNimBLEService.get());

    When(Method(mockNimBLEService, getServer)).AlwaysReturn(&mockNimBLEServer.get());
    Fake(Method(mockNimBLEService, start));

    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 1U>)));
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const unsigned short)));
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::string)));

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
    When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(23);

    When(Method(mockDeltaTimesCharacteristic, getSubscribedCount)).AlwaysReturn(1);
    When(Method(mockDeltaTimesCharacteristic, getService)).AlwaysReturn(&mockNimBLEService.get());
    Fake(Method(mockDeltaTimesCharacteristic, notify));
    When(
        OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
            .Using(CommonBleFlags::deltaTimesUuid, Any()))
        .AlwaysReturn(&mockDeltaTimesCharacteristic.get());
    When(
        OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
            .Using(CommonBleFlags::handleForcesUuid, Any()))
        .AlwaysReturn(&mockDeltaTimesCharacteristic.get());
    Fake(Method(mockDeltaTimesCharacteristic, notify));

    BluetoothService bluetoothService(mockEEPROMService.get(), mockSdCardService.get(), mockOtaService.get());
    bluetoothService.setup();
    NimBLECharacteristicCallbacks *chunkedNotifyMetricCallback = std::move(mockDeltaTimesCharacteristic.get().callbacks);

    ble_gap_conn_desc first = {0};
    ble_gap_conn_desc second = {1};

    SECTION("should ignore subscription if its not to delta times characteristic or handle force characteristic")
    {
        When(Method(mockDeltaTimesCharacteristic, getUUID)).AlwaysReturn(NimBLEUUID{CommonBleFlags::extendedMetricsUuid});

        chunkedNotifyMetricCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &first, 0);
        chunkedNotifyMetricCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &second, 0);

        bluetoothService.getDeltaTimesMTU();

        Verify(Method(mockNimBLEServer, getPeerMTU)).Never();
    }

    SECTION("should add new connection's client ID")
    {
        SECTION("to deltaTime client ID list when connection is made to delta times")
        {
            When(Method(mockDeltaTimesCharacteristic, getUUID)).AlwaysReturn(NimBLEUUID{CommonBleFlags::deltaTimesUuid});

            chunkedNotifyMetricCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &first, 0);
            chunkedNotifyMetricCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &second, 0);

            bluetoothService.getDeltaTimesMTU();

            Verify(Method(mockNimBLEServer, getPeerMTU)).Exactly(2);
        }

        SECTION("to deltaTime client ID list when connection is made to handle forces")
        {
            Fake(OverloadedMethod(mockDeltaTimesCharacteristic, setValue, void(const unsigned char *data, size_t length)));
            When(Method(mockDeltaTimesCharacteristic, getUUID)).AlwaysReturn(NimBLEUUID{CommonBleFlags::handleForcesUuid});
            Fake(Method(mockArduino, xTaskCreatePinnedToCore));
            Fake(Method(mockArduino, vTaskDelete));

            chunkedNotifyMetricCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &first, 0);
            chunkedNotifyMetricCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &second, 0);

            bluetoothService.notifyHandleForces({30.1});

            Verify(Method(mockNimBLEServer, getPeerMTU)).Exactly(2);
        }
    }
}
// NOLINTEND(readability-magic-numbers)