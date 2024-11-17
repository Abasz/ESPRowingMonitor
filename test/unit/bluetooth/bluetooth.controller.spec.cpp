// NOLINTBEGIN(readability-magic-numbers)
#include <array>
#include <string>

#include "../include/catch_amalgamated.hpp"
#include "../include/fakeit.hpp"

#include "../include/Arduino.h"
#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/ble-services/base-metrics.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/battery.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/device-info.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/ota.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/settings.service.interface.h"
#include "../../../src/peripherals/bluetooth/bluetooth.controller.h"
#include "../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../src/utils/configuration.h"
#include "../../../src/utils/enums.h"
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("BluetoothController", "[peripheral]")
{
    const auto serviceFlag = BleServiceFlag::CpsService;

    Mock<IEEPROMService> mockEEPROMService;
    Mock<IOtaUpdaterService> mockOtaUpdaterService;
    Mock<ISettingsBleService> mockSettingsBleService;
    Mock<IBatteryBleService> mockBatteryBleService;
    Mock<IDeviceInfoBleService> mockDeviceInfoBleService;
    Mock<IOtaBleService> mockOtaBleService;
    Mock<IBaseMetricsBleService> mockBaseMetricsBleService;

    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();
    mockNimBLECharacteristic.Reset();

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockNimBLEService.get());
    Fake(Method(mockNimBLEServer, createServer));
    Fake(Method(mockNimBLEServer, init));
    Fake(Method(mockNimBLEServer, setPower));
    Fake(Method(mockNimBLEServer, start));

    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))).AlwaysReturn(&mockNimBLECharacteristic.get());

    When(Method(mockBatteryBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockSettingsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockDeviceInfoBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockOtaBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockOtaBleService, getOtaTx)).AlwaysReturn(&mockNimBLECharacteristic.get());
    When(Method(mockBaseMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    Fake(Method(mockNimBLEService, start));

    Fake(Method(mockNimBLECharacteristic, setCallbacks));

    Fake(Method(mockNimBLEAdvertising, start));
    Fake(Method(mockNimBLEAdvertising, stop));
    Fake(Method(mockNimBLEAdvertising, setAppearance));
    Fake(Method(mockNimBLEAdvertising, addServiceUUID));

    When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(serviceFlag);

    Fake(Method(mockOtaUpdaterService, begin));

    BluetoothController bluetoothController(mockEEPROMService.get(), mockOtaUpdaterService.get(), mockSettingsBleService.get(), mockBatteryBleService.get(), mockDeviceInfoBleService.get(), mockOtaBleService.get(), mockBaseMetricsBleService.get());

    SECTION("startBLEServer method should start advertisement")
    {
        bluetoothController.startBLEServer();

        Verify(Method(mockNimBLEAdvertising, start)).Once();
    }

    SECTION("stopServer method should start advertisement")
    {
        bluetoothController.stopServer();

        Verify(Method(mockNimBLEAdvertising, stop)).Once();
    }

    SECTION("setup method")
    {
        SECTION("should initialize BLE with correct device name")
        {
            mockNimBLEServer.ClearInvocationHistory();

            auto cpsDeviceName = Configurations::deviceName;
            cpsDeviceName.append(" (CPS)");
            auto cscDeviceName = Configurations::deviceName;
            cscDeviceName.append(" (CSC)");

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);

            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, init).Using(cpsDeviceName)).Once();

            mockNimBLEServer.ClearInvocationHistory();
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);

            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, init).Using(cscDeviceName)).Once();
        }

        SECTION("should set BLE power")
        {
            mockNimBLEServer.ClearInvocationHistory();

            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, setPower).Using(static_cast<esp_power_level_t>(Configurations::bleSignalStrength), ESP_BLE_PWR_TYPE_ADV)).Once();
            Verify(Method(mockNimBLEServer, setPower).Using(static_cast<esp_power_level_t>(Configurations::bleSignalStrength), ESP_BLE_PWR_TYPE_DEFAULT)).Once();
        }

        SECTION("should create server")
        {
            mockNimBLEServer.ClearInvocationHistory();

            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, createServer)).Once();
        }

        SECTION("should set server callback function")
        {
            mockNimBLEServer.ClearInvocationHistory();

            bluetoothController.setup();

            REQUIRE(mockNimBLEServer.get().callbacks != nullptr);
            Verify(Method(mockNimBLEServer, createServer)).Once();
        }

        SECTION("should create battery service and related characteristics")
        {
            mockNimBLEServer.ClearInvocationHistory();
            Mock<NimBLEService> mockBatteryNimBLEService;

            When(Method(mockSettingsBleService, setup)).AlwaysReturn(&mockBatteryNimBLEService.get());
            Fake(Method(mockBatteryNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockSettingsBleService, setup).Using(Ne(nullptr)));
            Verify(Method(mockBatteryNimBLEService, start)).Once();
        }

        SECTION("should setup base metrics service")
        {
            mockNimBLEServer.ClearInvocationHistory();
            Mock<NimBLEService> mockBaseMetricsNimBLEService;
            When(Method(mockBaseMetricsBleService, setup)).AlwaysReturn(&mockBaseMetricsNimBLEService.get());
            Fake(Method(mockBaseMetricsNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockBaseMetricsBleService, setup).Using(Ne(nullptr), serviceFlag));
            Verify(Method(mockBaseMetricsNimBLEService, start)).Once();
        }

        SECTION("should setup extended BLE metrics service")
        {
            const unsigned int expectedProperty = NIMBLE_PROPERTY::NOTIFY;

            mockNimBLEServer.ClearInvocationHistory();
            mockNimBLECharacteristic.ClearInvocationHistory();

            Mock<NimBLEService> mockExtendedService;
            Mock<NimBLECharacteristic> mockDeltaTimesCharacteristic;
            Mock<NimBLECharacteristic> mockHandleForcesCharacteristic;

            When(Method(mockDeltaTimesCharacteristic, setCallbacks)).Do([&mockDeltaTimesCharacteristic](NimBLECharacteristicCallbacks *callbacks)
                                                                        { mockDeltaTimesCharacteristic.get().callbacks = callbacks; });
            When(Method(mockHandleForcesCharacteristic, setCallbacks)).Do([&mockHandleForcesCharacteristic](NimBLECharacteristicCallbacks *callbacks)
                                                                          { mockHandleForcesCharacteristic.get().callbacks = callbacks; });
            When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string)).Using(CommonBleFlags::extendedMetricsServiceUuid)).AlwaysReturn(&mockExtendedService.get());
            When(
                OverloadedMethod(mockExtendedService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::deltaTimesUuid, expectedProperty))
                .AlwaysReturn(&mockDeltaTimesCharacteristic.get());
            When(
                OverloadedMethod(mockExtendedService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::handleForcesUuid, expectedProperty))
                .AlwaysReturn(&mockHandleForcesCharacteristic.get());
            When(
                OverloadedMethod(mockExtendedService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::extendedMetricsUuid, expectedProperty))
                .AlwaysReturn(&mockNimBLECharacteristic.get());
            Fake(Method(mockExtendedService, start));

            bluetoothController.setup();

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string)).Using(CommonBleFlags::extendedMetricsServiceUuid)).Once();
            Verify(
                OverloadedMethod(mockExtendedService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::extendedMetricsUuid, expectedProperty))
                .Once();
            Verify(
                OverloadedMethod(mockExtendedService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::handleForcesUuid, expectedProperty))
                .Once();
            Verify(
                OverloadedMethod(mockExtendedService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::deltaTimesUuid, expectedProperty))
                .Once();

            REQUIRE(mockDeltaTimesCharacteristic.get().callbacks != nullptr);
            REQUIRE(mockHandleForcesCharacteristic.get().callbacks != nullptr);

            Verify(Method(mockExtendedService, start)).Once();
        }

        SECTION("should setup settings service")
        {
            mockNimBLEServer.ClearInvocationHistory();
            Mock<NimBLEService> mockSettingsNimBLEService;
            When(Method(mockSettingsBleService, setup)).AlwaysReturn(&mockSettingsNimBLEService.get());
            Fake(Method(mockSettingsNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockSettingsBleService, setup).Using(Ne(nullptr)));
            Verify(Method(mockSettingsNimBLEService, start)).Once();
        }

        SECTION("should start OTA")
        {
            mockNimBLEServer.ClearInvocationHistory();

            Mock<NimBLEService> mockOtaService;
            Mock<NimBLECharacteristic> mockTxCharacteristic;
            Mock<NimBLECharacteristic> mockRxCharacteristic;

            When(Method(mockOtaBleService, setup)).AlwaysReturn(&mockOtaService.get());
            Fake(Method(mockOtaService, start));

            bluetoothController.setup();

            SECTION("BLE service")
            {
                Verify(Method(mockOtaBleService, setup)).Once();
                Verify(Method(mockNimBLEServer, start)).Once();
            }
            SECTION("service with the OtaBleService txCharacteristic")
            {
                Verify(Method(mockOtaUpdaterService, begin)).Once();
                Verify(Method(mockOtaBleService, getOtaTx)).Once();
            }
        }

        SECTION("should setup device information service")
        {
            mockNimBLEServer.ClearInvocationHistory();
            Mock<NimBLEService> mockDeviceInfoNimBLEService;
            When(Method(mockDeviceInfoBleService, setup)).AlwaysReturn(&mockDeviceInfoNimBLEService.get());
            Fake(Method(mockDeviceInfoNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockDeviceInfoBleService, setup)).Once();
            Verify(Method(mockDeviceInfoNimBLEService, start)).Once();
        }

        SECTION("should start server")
        {
            mockNimBLEServer.ClearInvocationHistory();

            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, start)).Once();
        }
    }

    SECTION("isAnyDeviceConnected method")
    {
        SECTION("should return true if at least one device is connected")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).Return(1);

            const auto isConnected = bluetoothController.isAnyDeviceConnected();

            REQUIRE(isConnected == true);
        }

        SECTION("should return false if no device is connected")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0);

            const auto isConnected = bluetoothController.isAnyDeviceConnected();

            REQUIRE(isConnected == false);
        }
    }

    SECTION("isDeltaTimesSubscribed method")
    {
        bluetoothController.setup();

        SECTION("should return true if at least one device is connected")
        {
            mockNimBLECharacteristic.ClearInvocationHistory();

            When(Method(mockNimBLECharacteristic, getSubscribedCount)).Return(1);

            const auto isSubscribed = bluetoothController.isDeltaTimesSubscribed();

            REQUIRE(isSubscribed == true);
        }

        SECTION("should return false if no device is connected")
        {
            When(Method(mockNimBLECharacteristic, getSubscribedCount)).Return(0);

            const auto isSubscribed = bluetoothController.isDeltaTimesSubscribed();

            REQUIRE(isSubscribed == false);
        }
    }

    SECTION("getDeltaTimesMTU method")
    {
        Mock<NimBLECharacteristic> mockDeltaTimesCharacteristic;
        When(Method(mockNimBLEService, getServer)).AlwaysReturn(&mockNimBLEServer.get());
        When(
            OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                .Using(CommonBleFlags::deltaTimesUuid, NIMBLE_PROPERTY::NOTIFY))
            .AlwaysReturn(&mockDeltaTimesCharacteristic.get());
        When(Method(mockDeltaTimesCharacteristic, getSubscribedCount)).AlwaysReturn(1);
        When(Method(mockDeltaTimesCharacteristic, getService)).AlwaysReturn(&mockNimBLEService.get());
        When(Method(mockDeltaTimesCharacteristic, getUUID)).AlwaysReturn(NimBLEUUID{CommonBleFlags::deltaTimesUuid});
        When(Method(mockDeltaTimesCharacteristic, setCallbacks)).Do([&mockDeltaTimesCharacteristic](NimBLECharacteristicCallbacks *callbacks)
                                                                    { mockDeltaTimesCharacteristic.get().callbacks = callbacks; });

        bluetoothController.setup();

        Verify(
            OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                .Using(CommonBleFlags::deltaTimesUuid, NIMBLE_PROPERTY::NOTIFY))
            .Once();

        SECTION("should return 0 when no device is connected")
        {
            const auto expectedMTU = 0;
            ble_gap_conn_desc first = {0};
            ble_gap_conn_desc second = {1};
            mockNimBLEServer.get().callbacks->onDisconnect(&mockNimBLEServer.get(), &first);
            mockNimBLEServer.get().callbacks->onDisconnect(&mockNimBLEServer.get(), &second);

            const auto mtu = bluetoothController.getDeltaTimesMTU();

            REQUIRE(mtu == expectedMTU);
        }

        SECTION("should return the lowest MTU")
        {
            const auto expectedMTU = 23;
            ble_gap_conn_desc first = {0};
            ble_gap_conn_desc second = {1};

            When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU, 100, 100);
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0, 1);

            mockDeltaTimesCharacteristic.get().callbacks->onSubscribe(&mockDeltaTimesCharacteristic.get(), &first, 0);
            mockDeltaTimesCharacteristic.get().callbacks->onSubscribe(&mockDeltaTimesCharacteristic.get(), &second, 0);

            const auto mtu = bluetoothController.getDeltaTimesMTU();

            REQUIRE(mtu == expectedMTU);
        }

        SECTION("should return 512 as MTU even if device reports higher")
        {
            const auto expectedMTU = 512;
            ble_gap_conn_desc first = {0};
            ble_gap_conn_desc second = {1};

            When(Method(mockNimBLEServer, getPeerMTU)).Return(1200, 1000);
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0, 1);
            When(Method(mockDeltaTimesCharacteristic, getSubscribedCount)).AlwaysReturn(1);

            mockDeltaTimesCharacteristic.get().callbacks->onSubscribe(&mockDeltaTimesCharacteristic.get(), &first, 0);
            mockDeltaTimesCharacteristic.get().callbacks->onSubscribe(&mockDeltaTimesCharacteristic.get(), &second, 0);

            const auto mtu = bluetoothController.getDeltaTimesMTU();

            REQUIRE(mtu == expectedMTU);
        }

        SECTION("should ignore zero MTU when calculating minimum")
        {
            const auto expectedMTU = 23;
            ble_gap_conn_desc first = {0};
            ble_gap_conn_desc second = {1};

            When(Method(mockNimBLEServer, getPeerMTU)).Return(0, expectedMTU, 100);
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0, 1);
            When(Method(mockDeltaTimesCharacteristic, getSubscribedCount)).AlwaysReturn(1);

            mockDeltaTimesCharacteristic.get().callbacks->onSubscribe(&mockDeltaTimesCharacteristic.get(), &first, 0);
            mockDeltaTimesCharacteristic.get().callbacks->onSubscribe(&mockDeltaTimesCharacteristic.get(), &second, 0);

            const auto mtu = bluetoothController.getDeltaTimesMTU();

            REQUIRE(mtu == expectedMTU);
        }
    }
}
// NOLINTEND(readability-magic-numbers)