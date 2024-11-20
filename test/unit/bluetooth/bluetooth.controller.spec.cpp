// NOLINTBEGIN(readability-magic-numbers)
#include "../include/catch_amalgamated.hpp"
#include "../include/fakeit.hpp"

#include "../include/Arduino.h"
#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/ble-services/base-metrics.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/battery.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/device-info.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/extended-metrics.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/ota.service.interface.h"
#include "../../../src/peripherals/bluetooth/ble-services/settings.service.interface.h"
#include "../../../src/peripherals/bluetooth/bluetooth.controller.h"
#include "../../../src/rower/stroke.model.h"
#include "../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../src/utils/configuration.h"
#include "../../../src/utils/enums.h"
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("BluetoothController", "[peripheral]")
{
    mockArduino.Reset();
    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();

    Mock<IEEPROMService> mockEEPROMService;
    Mock<IOtaUpdaterService> mockOtaUpdaterService;
    Mock<ISettingsBleService> mockSettingsBleService;
    Mock<IBatteryBleService> mockBatteryBleService;
    Mock<IDeviceInfoBleService> mockDeviceInfoBleService;
    Mock<IOtaBleService> mockOtaBleService;
    Mock<IBaseMetricsBleService> mockBaseMetricsBleService;
    Mock<IExtendedMetricBleService> mockExtendedMetricsBleService;

    const auto serviceFlag = BleServiceFlag::CpsService;
    const RowingDataModels::RowingMetrics expectedData{
        .distance = 100,
        .lastRevTime = 2000,
        .lastStrokeTime = 1600,
        .strokeCount = 10,
        .driveDuration = 1001,
        .recoveryDuration = 1003,
        .avgStrokePower = 70,
        .dragCoefficient = 0.00001,
        .driveHandleForces = {1.1, 2.2, 100.1},
    };

    Fake(Method(mockNimBLEServer, createServer));
    Fake(Method(mockNimBLEServer, init));
    Fake(Method(mockNimBLEServer, setPower));
    Fake(Method(mockNimBLEServer, start));

    When(Method(mockBatteryBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockSettingsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockDeviceInfoBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockOtaBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockOtaBleService, getOtaTx)).AlwaysReturn(&mockNimBLECharacteristic.get());

    When(Method(mockBaseMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockBaseMetricsBleService, isSubscribed)).AlwaysReturn(false);

    When(Method(mockExtendedMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).AlwaysReturn({});
    When(Method(mockExtendedMetricsBleService, isExtendedMetricsSubscribed)).AlwaysReturn(false);

    Fake(Method(mockNimBLEService, start));

    Fake(Method(mockNimBLEAdvertising, start));
    Fake(Method(mockNimBLEAdvertising, stop));
    Fake(Method(mockNimBLEAdvertising, setAppearance));
    Fake(Method(mockNimBLEAdvertising, addServiceUUID));

    When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(serviceFlag);

    Fake(Method(mockOtaUpdaterService, begin));

    BluetoothController bluetoothController(mockEEPROMService.get(), mockOtaUpdaterService.get(), mockSettingsBleService.get(), mockBatteryBleService.get(), mockDeviceInfoBleService.get(), mockOtaBleService.get(), mockBaseMetricsBleService.get(), mockExtendedMetricsBleService.get());

    SECTION("update method")
    {
        When(Method(mockArduino, millis)).Return(0);
        bluetoothController.notifyNewMetrics(expectedData);
        mockBaseMetricsBleService.ClearInvocationHistory();

        SECTION("when there are subscribers and when last notification was send more than 1 seconds ago should notify with last available base metric ")
        {
            const auto bleFlag = BleServiceFlag::CpsService;

            const auto secInMicroSec = 1e6L;
            const unsigned short bleRevTimeData = lroundl((expectedData.lastRevTime / secInMicroSec) * (bleFlag == BleServiceFlag::CpsService ? 2'048 : 1'024)) % USHRT_MAX;
            const unsigned int bleRevCountData = lround(expectedData.distance);
            const unsigned short bleStrokeTimeData = lroundl((expectedData.lastStrokeTime / secInMicroSec) * 1'024) % USHRT_MAX;
            const unsigned short bleStrokeCountData = expectedData.strokeCount;
            const short bleAvgStrokePowerData = static_cast<short>(lround(expectedData.avgStrokePower));

            When(Method(mockArduino, millis)).Return(1001);
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(bleFlag);
            When(Method(mockBaseMetricsBleService, isSubscribed)).Return(true);
            Fake(Method(mockBaseMetricsBleService, broadcastBaseMetrics));

            bluetoothController.update();

            Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics).Using(Eq(bleRevTimeData), Eq(bleRevCountData), Eq(bleStrokeTimeData), Eq(bleStrokeCountData), Eq(bleAvgStrokePowerData))).Once();
        }

        SECTION("not notify base metric when last notification was send less than 1 seconds ago")
        {
            When(Method(mockArduino, millis)).Return(999);
            When(Method(mockBaseMetricsBleService, isSubscribed)).Return(true);

            bluetoothController.update();

            Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)).Never();
        }

        SECTION("not notify base metric when there are no subscribers")
        {
            When(Method(mockArduino, millis)).Return(1001);
            When(Method(mockBaseMetricsBleService, isSubscribed)).Return(false);

            bluetoothController.update();

            Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)).Never();
        }
    }

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
            auto cpsDeviceName = Configurations::deviceName;
            cpsDeviceName.append(" (CPS)");
            auto cscDeviceName = Configurations::deviceName;
            cscDeviceName.append(" (CSC)");

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);

            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, init).Using(cpsDeviceName)).Once();

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);

            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, init).Using(cscDeviceName)).Once();
        }

        SECTION("should set BLE power")
        {
            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, setPower).Using(static_cast<esp_power_level_t>(Configurations::bleSignalStrength), ESP_BLE_PWR_TYPE_ADV)).Once();
            Verify(Method(mockNimBLEServer, setPower).Using(static_cast<esp_power_level_t>(Configurations::bleSignalStrength), ESP_BLE_PWR_TYPE_DEFAULT)).Once();
        }

        SECTION("should create server")
        {
            bluetoothController.setup();

            Verify(Method(mockNimBLEServer, createServer)).Once();
        }

        SECTION("should set server callback function")
        {
            bluetoothController.setup();

            REQUIRE(mockNimBLEServer.get().callbacks != nullptr);
            Verify(Method(mockNimBLEServer, createServer)).Once();
        }

        SECTION("should create battery service and related characteristics")
        {
            Mock<NimBLEService> mockBatteryNimBLEService;

            When(Method(mockSettingsBleService, setup)).AlwaysReturn(&mockBatteryNimBLEService.get());
            Fake(Method(mockBatteryNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockSettingsBleService, setup).Using(Ne(nullptr)));
            Verify(Method(mockBatteryNimBLEService, start)).Once();
        }

        SECTION("should setup base metrics service")
        {
            Mock<NimBLEService> mockBaseMetricsNimBLEService;

            When(Method(mockBaseMetricsBleService, setup)).AlwaysReturn(&mockBaseMetricsNimBLEService.get());
            Fake(Method(mockBaseMetricsNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockBaseMetricsBleService, setup).Using(Ne(nullptr), serviceFlag));
            Verify(Method(mockBaseMetricsNimBLEService, start)).Once();
        }

        SECTION("should setup extended BLE metrics service")
        {
            Mock<NimBLEService> mockExtendedMetricsNimBLEService;

            When(Method(mockExtendedMetricsBleService, setup)).AlwaysReturn(&mockExtendedMetricsNimBLEService.get());
            Fake(Method(mockExtendedMetricsNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockExtendedMetricsBleService, setup)).Once();
            Verify(Method(mockExtendedMetricsNimBLEService, start)).Once();
        }

        SECTION("should setup settings service")
        {
            Mock<NimBLEService> mockSettingsNimBLEService;

            When(Method(mockSettingsBleService, setup)).AlwaysReturn(&mockSettingsNimBLEService.get());
            Fake(Method(mockSettingsNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockSettingsBleService, setup).Using(Ne(nullptr)));
            Verify(Method(mockSettingsNimBLEService, start)).Once();
        }

        SECTION("should start OTA")
        {
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
            Mock<NimBLEService> mockDeviceInfoNimBLEService;

            When(Method(mockDeviceInfoBleService, setup)).AlwaysReturn(&mockDeviceInfoNimBLEService.get());
            Fake(Method(mockDeviceInfoNimBLEService, start));

            bluetoothController.setup();

            Verify(Method(mockDeviceInfoBleService, setup)).Once();
            Verify(Method(mockDeviceInfoNimBLEService, start)).Once();
        }

        SECTION("should start server")
        {
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

    SECTION("calculateDeltaTimesMtu method")
    {
        std::vector<unsigned char> clientIds{0, 1};
        When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).Return(clientIds);

        SECTION("should return 0 when no device is connected")
        {
            const auto expectedMTU = 0;
            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).Return({});

            const auto mtu = bluetoothController.calculateDeltaTimesMtu();

            REQUIRE(mtu == expectedMTU);
        }

        SECTION("should return the lowest MTU")
        {
            const auto expectedMTU = 23;

            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientMtu)).Return(expectedMTU, 100);

            const auto mtu = bluetoothController.calculateDeltaTimesMtu();

            REQUIRE(mtu == expectedMTU);
        }

        SECTION("should return 512 as MTU even if device reports higher")
        {
            const auto expectedMTU = 512;

            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientMtu)).Return(1200, 1000);

            const auto mtu = bluetoothController.calculateDeltaTimesMtu();

            REQUIRE(mtu == expectedMTU);
        }

        SECTION("should ignore zero MTU when calculating minimum")
        {
            const auto expectedMTU = 23;

            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientMtu)).Return(0, expectedMTU);

            const auto mtu = bluetoothController.calculateDeltaTimesMtu();

            REQUIRE(mtu == expectedMTU);
        }
    }
}
// NOLINTEND(readability-magic-numbers)