// NOLINTBEGIN(readability-magic-numbers)
#include <array>
#include <string>

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

TEST_CASE("BluetoothController ServerCallbacks", "[callbacks]")
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

    BluetoothController bluetoothController(mockEEPROMService.get(), mockSdCardService.get(), mockOtaService.get());
    bluetoothController.setup();
    mockNimBLEAdvertising.ClearInvocationHistory();
    NimBLEServerCallbacks *serverCallback = std::move(mockNimBLEServer.get().callbacks);
    NimBLECharacteristicCallbacks *deltaTimesCallback = std::move(mockDeltaTimesCharacteristic.get().callbacks);

    SECTION("onConnect method should")
    {
        SECTION("restart advertising when connection count is less than 2")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0, 1, 2);

            serverCallback->onConnect(&mockNimBLEServer.get());
            serverCallback->onConnect(&mockNimBLEServer.get());

            Verify(Method(mockNimBLEAdvertising, start)).Exactly(2);
        }

        SECTION("should not restart advertising when connection count is 2 or more")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).AlwaysReturn(2);

            serverCallback->onConnect(&mockNimBLEServer.get());

            Verify(Method(mockNimBLEAdvertising, start)).Never();
        }
    }

    SECTION("onDisconnect method should")
    {
        ble_gap_conn_desc first = {0};
        ble_gap_conn_desc second = {1};

        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        Fake(OverloadedMethod(mockDeltaTimesCharacteristic, setValue, void(const unsigned char *data, size_t length)));
        When(Method(mockDeltaTimesCharacteristic, getUUID)).Return(NimBLEUUID{CommonBleFlags::deltaTimesUuid}, NimBLEUUID{CommonBleFlags::deltaTimesUuid}, NimBLEUUID{CommonBleFlags::handleForcesUuid}, NimBLEUUID{CommonBleFlags::handleForcesUuid}, NimBLEUUID{CommonBleFlags::deltaTimesUuid}, NimBLEUUID{CommonBleFlags::deltaTimesUuid}, NimBLEUUID{CommonBleFlags::handleForcesUuid}, NimBLEUUID{CommonBleFlags::handleForcesUuid});

        deltaTimesCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &first, 0);
        deltaTimesCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &second, 0);
        deltaTimesCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &first, 0);
        deltaTimesCallback->onSubscribe(&mockDeltaTimesCharacteristic.get(), &second, 0);

        bluetoothController.getDeltaTimesMTU();
        bluetoothController.notifyHandleForces({30.3});

        Verify(Method(mockNimBLEServer, getPeerMTU)).Exactly(4);

        SECTION("should remove clientID if exists in the deltaTimes and handleForces ID list")
        {
            mockNimBLEServer.ClearInvocationHistory();

            serverCallback->onDisconnect(&mockNimBLEServer.get(), &first);

            bluetoothController.getDeltaTimesMTU();
            bluetoothController.notifyHandleForces({30.3});

            Verify(Method(mockNimBLEServer, getPeerMTU)).Exactly(2);
        }

        SECTION("should not remove clientID if does not exists in any list")
        {
            ble_gap_conn_desc third = {3};

            mockNimBLEServer.ClearInvocationHistory();

            serverCallback->onDisconnect(&mockNimBLEServer.get(), &third);

            bluetoothController.getDeltaTimesMTU();
            bluetoothController.notifyHandleForces({30.3});

            Verify(Method(mockNimBLEServer, getPeerMTU)).Exactly(4);
        }
    }
}
// NOLINTEND(readability-magic-numbers)