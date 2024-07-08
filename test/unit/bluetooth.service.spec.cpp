// NOLINTBEGIN(readability-magic-numbers)
#include <array>
#include <string>

#include "catch_amalgamated.hpp"
#include "fakeit.hpp"

#include "Arduino.h"
#include "NimBLEDevice.h"

#include "../../src/peripherals/bluetooth.service.h"
#include "../../src/peripherals/sd-card.service.interface.h"
#include "../../src/utils/EEPROM.service.interface.h"
#include "../../src/utils/configuration.h"
#include "../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("BluetoothServer", "[peripheral]")
{
    const auto serviceFlag = BleServiceFlag::CpsService;
    const auto logToBluetooth = true;
    const auto logToSdCard = true;
    const auto logLevel = ArduinoLogLevel::LogLevelSilent;
    const auto logFileOpen = true;

    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISdCardService> mockSdCardService;

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

    Fake(Method(mockNimBLEService, start));

    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 1U>)));
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const unsigned short)));
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::string)));

    Fake(Method(mockNimBLEAdvertising, start));
    Fake(Method(mockNimBLEAdvertising, stop));
    Fake(Method(mockNimBLEAdvertising, setAppearance));
    Fake(Method(mockNimBLEAdvertising, addServiceUUID));

    When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(serviceFlag);
    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetooth);
    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);

    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

    BluetoothService bluetoothService(mockEEPROMService.get(), mockSdCardService.get());

    SECTION("startBLEServer method should start advertisement")
    {
        bluetoothService.startBLEServer();

        Verify(Method(mockNimBLEAdvertising, start)).Once();
    }

    SECTION("stopServer method should start advertisement")
    {
        bluetoothService.stopServer();

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

            bluetoothService.setup();

            Verify(Method(mockNimBLEServer, init).Using(cpsDeviceName)).Once();

            mockNimBLEServer.ClearInvocationHistory();
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);

            bluetoothService.setup();

            Verify(Method(mockNimBLEServer, init).Using(cscDeviceName)).Once();
        }

        SECTION("should set BLE power")
        {
            mockNimBLEServer.ClearInvocationHistory();

            bluetoothService.setup();

            Verify(Method(mockNimBLEServer, setPower).Using(static_cast<esp_power_level_t>(Configurations::bleSignalStrength), ESP_BLE_PWR_TYPE_ADV)).Once();
            Verify(Method(mockNimBLEServer, setPower).Using(static_cast<esp_power_level_t>(Configurations::bleSignalStrength), ESP_BLE_PWR_TYPE_DEFAULT)).Once();
        }

        SECTION("should create server")
        {
            mockNimBLEServer.ClearInvocationHistory();

            bluetoothService.setup();

            Verify(Method(mockNimBLEServer, createServer)).Once();
        }

        SECTION("should set server callback function")
        {
            mockNimBLEServer.ClearInvocationHistory();

            bluetoothService.setup();

            REQUIRE(mockNimBLEServer.get().callbacks != nullptr);
            Verify(Method(mockNimBLEServer, createServer)).Once();
        }

        SECTION("should create battery service and related characteristics")
        {
            mockNimBLEServer.ClearInvocationHistory();
            const unsigned int expectedNimBLEProperty = NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY;

            Mock<NimBLEService> mockBatteryService;
            When(
                OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short))
                    .Using(CommonBleFlags::batterySvcUuid))
                .AlwaysReturn(&mockBatteryService.get());
            When(
                OverloadedMethod(mockBatteryService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::batteryLevelUuid, expectedNimBLEProperty))
                .AlwaysReturn(&mockNimBLECharacteristic.get());
            Fake(Method(mockBatteryService, start));

            bluetoothService.setup();

            Verify(
                OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short))
                    .Using(CommonBleFlags::batterySvcUuid))
                .Once();
            Verify(
                OverloadedMethod(mockBatteryService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::batteryLevelUuid, expectedNimBLEProperty))
                .Once();
            Verify(Method(mockBatteryService, start)).Once();
        }

        SECTION("should select measurement service (CPS vs. CSC) based on settings")
        {
            mockNimBLEServer.ClearInvocationHistory();

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);

            bluetoothService.setup();

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);

            bluetoothService.setup();

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(PSCSensorBleFlags::cyclingPowerSvcUuid)).Once();
            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid)).Once();
        }

        SECTION("should correctly setup CPS service")
        {
            const unsigned int expectedMeasurementProperty = NIMBLE_PROPERTY::NOTIFY;
            const unsigned int expectedFlagsProperty = NIMBLE_PROPERTY::READ;
            const unsigned int expectedControlPointProperty = NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE;

            mockNimBLEServer.ClearInvocationHistory();

            Mock<NimBLEService> mockCpsService;
            Mock<NimBLECharacteristic> mockCpsCharacteristic;

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);
            When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(PSCSensorBleFlags::cyclingPowerSvcUuid)).AlwaysReturn(&mockCpsService.get());
            When(OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockCpsCharacteristic.get());
            Fake(OverloadedMethod(mockCpsCharacteristic, setValue, void(const unsigned short)));
            Fake(Method(mockCpsService, start));

            bluetoothService.setup();

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (unsigned short)).Using(PSCSensorBleFlags::cyclingPowerSvcUuid)).Once();
            Verify(
                OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(PSCSensorBleFlags::pscMeasurementUuid, expectedMeasurementProperty))
                .Once();
            Verify(
                OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(PSCSensorBleFlags::pscFeatureUuid, expectedFlagsProperty))
                .Once();
            Verify(
                OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::sensorLocationUuid, expectedFlagsProperty))
                .Once();
            Verify(
                OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(PSCSensorBleFlags::pscControlPointUuid, expectedControlPointProperty))
                .Once();

            Verify(OverloadedMethod(mockCpsCharacteristic, setValue, void(const unsigned short)).Using(PSCSensorBleFlags::pscFeaturesFlag)).Once();
            Verify(OverloadedMethod(mockCpsCharacteristic, setValue, void(const unsigned short)).Using(CommonBleFlags::sensorLocationFlag)).Once();

            REQUIRE(mockCpsCharacteristic.get().callbacks != nullptr);

            Verify(Method(mockCpsService, start)).Once();
        }

        SECTION("should correctly setup CSC service")
        {
            const unsigned int expectedMeasurementProperty = NIMBLE_PROPERTY::NOTIFY;
            const unsigned int expectedFlagsProperty = NIMBLE_PROPERTY::READ;
            const unsigned int expectedControlPointProperty = NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE;

            mockNimBLEServer.ClearInvocationHistory();

            Mock<NimBLEService> mockCscService;
            Mock<NimBLECharacteristic> mockCscCharacteristic;

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);
            When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid)).AlwaysReturn(&mockCscService.get());
            When(OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockCscCharacteristic.get());
            Fake(OverloadedMethod(mockCscCharacteristic, setValue, void(const unsigned short)));
            Fake(Method(mockCscService, start));

            bluetoothService.setup();

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (unsigned short)).Using(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid)).Once();
            Verify(
                OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CSCSensorBleFlags::cscMeasurementUuid, expectedMeasurementProperty))
                .Once();
            Verify(
                OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CSCSensorBleFlags::cscFeatureUuid, expectedFlagsProperty))
                .Once();
            Verify(
                OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::sensorLocationUuid, expectedFlagsProperty))
                .Once();
            Verify(
                OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CSCSensorBleFlags::cscControlPointUuid, expectedControlPointProperty))
                .Once();

            Verify(OverloadedMethod(mockCscCharacteristic, setValue, void(const unsigned short)).Using(CSCSensorBleFlags::cscFeaturesFlag)).Once();
            Verify(OverloadedMethod(mockCscCharacteristic, setValue, void(const unsigned short)).Using(CommonBleFlags::sensorLocationFlag)).Once();

            REQUIRE(mockCscCharacteristic.get().callbacks != nullptr);

            Verify(Method(mockCscService, start)).Once();
        }

        SECTION("should setup extended BLE metrics service")
        {
            const unsigned int expectedProperty = NIMBLE_PROPERTY::NOTIFY;

            mockNimBLEServer.ClearInvocationHistory();
            mockNimBLECharacteristic.ClearInvocationHistory();

            Mock<NimBLEService> mockExtendedService;
            Mock<NimBLECharacteristic> mockDeltaTimesCharacteristic;
            Mock<NimBLECharacteristic> mockHandleForcesCharacteristic;

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

            bluetoothService.setup();

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
            const unsigned int expectedSettingsProperty = NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ;
            const unsigned int expectedControlPointProperty = NIMBLE_PROPERTY::INDICATE | NIMBLE_PROPERTY::WRITE;
            const unsigned char expectedSettings =
                ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
                (static_cast<unsigned char>(logLevel) << 4U);

            mockNimBLEServer.ClearInvocationHistory();

            Mock<NimBLEService> mockSettingsService;
            Mock<NimBLECharacteristic> mockSettingsCharacteristic;

            When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string)).Using(CommonBleFlags::settingsServiceUuid)).AlwaysReturn(&mockSettingsService.get());
            When(
                OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::settingsControlPointUuid, expectedControlPointProperty))
                .AlwaysReturn(&mockSettingsCharacteristic.get());
            When(
                OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::settingsUuid, expectedSettingsProperty))
                .AlwaysReturn(&mockSettingsCharacteristic.get());
            Fake(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>)));
            Fake(Method(mockSettingsService, start));

            bluetoothService.setup();

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string)).Using(CommonBleFlags::settingsServiceUuid)).Once();
            Verify(
                OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::settingsUuid, expectedSettingsProperty))
                .Once();
            Verify(
                OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::settingsControlPointUuid, expectedControlPointProperty))
                .Once();
            Verify(
                OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                    .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                .Once();

            REQUIRE(mockSettingsCharacteristic.get().callbacks != nullptr);

            Verify(Method(mockSettingsService, start)).Once();
        }

        SECTION("should setup device information service")
        {
            const unsigned int expectedProperty = NIMBLE_PROPERTY::READ;

            mockNimBLEServer.ClearInvocationHistory();

            Mock<NimBLEService> mockDeviceInfoService;
            Mock<NimBLECharacteristic> mockDeviceInfoCharacteristic;

            When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(CommonBleFlags::deviceInfoSvcUuid)).AlwaysReturn(&mockDeviceInfoService.get());
            When(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int)))
                .AlwaysReturn(&mockDeviceInfoCharacteristic.get());
            Fake(OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string)));
            Fake(Method(mockDeviceInfoService, start));

            bluetoothService.setup();

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(CommonBleFlags::deviceInfoSvcUuid)).Once();
            Verify(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::manufacturerNameSvcUuid, expectedProperty))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::modelNumberSvcUuid, expectedProperty))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::serialNumberSvcUuid, expectedProperty))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::softwareNumberSvcUuid, expectedProperty))
                .Once();

            Verify(
                OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string))
                    .Using(Configurations::deviceName))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string))
                    .Using(Configurations::modelNumber))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string))
                    .Using(Configurations::serialNumber))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string))
                    .Using(Configurations::softwareVersion))
                .Once();

            Verify(Method(mockDeviceInfoService, start)).Once();
        }

        SECTION("should start server")
        {
            mockNimBLEServer.ClearInvocationHistory();

            bluetoothService.setup();

            Verify(Method(mockNimBLEServer, start)).Once();
        }
    }

    SECTION("isAnyDeviceConnected method")
    {
        SECTION("should return true if at least one device is connected")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).Return(1);

            const auto isConnected = bluetoothService.isAnyDeviceConnected();

            REQUIRE(isConnected == true);
        }

        SECTION("should return false if no device is connected")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0);

            const auto isConnected = bluetoothService.isAnyDeviceConnected();

            REQUIRE(isConnected == false);
        }
    }

    SECTION("isDeltaTimesSubscribed method")
    {
        bluetoothService.setup();

        SECTION("should return true if at least one device is connected")
        {
            mockNimBLECharacteristic.ClearInvocationHistory();

            When(Method(mockNimBLECharacteristic, getSubscribedCount)).Return(1);

            const auto isSubscribed = bluetoothService.isDeltaTimesSubscribed();

            REQUIRE(isSubscribed == true);
        }

        SECTION("should return false if no device is connected")
        {
            When(Method(mockNimBLECharacteristic, getSubscribedCount)).Return(0);

            const auto isSubscribed = bluetoothService.isDeltaTimesSubscribed();

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

        bluetoothService.setup();

        Verify(
            OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                .Using(CommonBleFlags::deltaTimesUuid, NIMBLE_PROPERTY::NOTIFY))
            .Once();

        SECTION("should return 0 when no device is connected")
        {
            const auto expectedMTU = 0;

            When(Method(mockDeltaTimesCharacteristic, getSubscribedCount)).AlwaysReturn(0);

            const auto mtu = bluetoothService.getDeltaTimesMTU();

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

            const auto mtu = bluetoothService.getDeltaTimesMTU();

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

            const auto mtu = bluetoothService.getDeltaTimesMTU();

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

            const auto mtu = bluetoothService.getDeltaTimesMTU();

            REQUIRE(mtu == expectedMTU);
        }
    }
}
// NOLINTEND(readability-magic-numbers)