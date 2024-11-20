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
    const auto minimumDeltaTimeMtu = 100;
    const RowingDataModels::RowingMetrics expectedData{
        .distance = 100,
        .lastRevTime = 2'000,
        .lastStrokeTime = 1'600,
        .strokeCount = 10,
        .driveDuration = 1'001,
        .recoveryDuration = 1'003,
        .avgStrokePower = 70,
        .dragCoefficient = 0.00001,
        .driveHandleForces = {1.1, 2.2, 100.1},
    };
    const std::vector<unsigned char> emptyClientIds{};

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
    When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).AlwaysReturn(emptyClientIds);
    When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn(emptyClientIds);
    When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(0);
    When(Method(mockExtendedMetricsBleService, isExtendedMetricsSubscribed)).AlwaysReturn(false);
    Fake(Method(mockExtendedMetricsBleService, broadcastDeltaTimes));

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
        const auto expectedDeltaTime = 10'000;

        When(Method(mockArduino, millis)).Return(0);
        bluetoothController.notifyNewMetrics(expectedData);
        mockBaseMetricsBleService.ClearInvocationHistory();

        SECTION("when there are subscribers and when last notification was send more than 1 seconds ago should notify with last available base metric")
        {
            const auto bleFlag = BleServiceFlag::CpsService;

            const auto secInMicroSec = 1e6L;
            const unsigned short bleRevTimeData = lroundl((expectedData.lastRevTime / secInMicroSec) * (bleFlag == BleServiceFlag::CpsService ? 2'048 : 1'024)) % USHRT_MAX;
            const unsigned int bleRevCountData = lround(expectedData.distance);
            const unsigned short bleStrokeTimeData = lroundl((expectedData.lastStrokeTime / secInMicroSec) * 1'024) % USHRT_MAX;
            const unsigned short bleStrokeCountData = expectedData.strokeCount;
            const short bleAvgStrokePowerData = static_cast<short>(lround(expectedData.avgStrokePower));

            When(Method(mockArduino, millis)).Return(1'001);
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
            When(Method(mockArduino, millis)).Return(1'001);
            When(Method(mockBaseMetricsBleService, isSubscribed)).Return(false);

            bluetoothController.update();

            Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)).Never();
        }

        SECTION("broadcast deltaTimes when last notification was send more than 1 seconds ago and MTU is above minimum, clear vector and reserve memory based on MTU")
        {
            const auto blinkInterval = Configurations::ledBlinkFrequency + 1U;

            std::vector<std::vector<unsigned long>> notifiedDeltaTimes{};
            When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval);
            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn({0});
            When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu);
            Fake(Method(mockExtendedMetricsBleService, broadcastDeltaTimes).Matching([&notifiedDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                                     {
                notifiedDeltaTimes.push_back(deltaTimes);

                return true; }));
            bluetoothController.notifyNewDeltaTime(expectedDeltaTime);

            bluetoothController.update();

            REQUIRE_THAT(notifiedDeltaTimes, Catch::Matchers::SizeIs(1));
            REQUIRE_THAT(notifiedDeltaTimes[0], Catch::Matchers::RangeEquals(std::vector<unsigned long>{expectedDeltaTime}));
            Verify(Method(mockExtendedMetricsBleService, broadcastDeltaTimes).Matching([](const std::vector<unsigned long> &deltaTimes)
                                                                                       { return deltaTimes.capacity() == minimumDeltaTimeMtu / sizeof(unsigned long) + 1U && deltaTimes.empty(); }))
                .Once();
        }

        SECTION("not broadcast deltaTimes when")
        {
            bluetoothController.notifyNewDeltaTime(expectedDeltaTime);

            SECTION("last notification was send less than 1 seconds ago")
            {
                When(Method(mockArduino, millis)).AlwaysReturn(999);
                When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu);
                When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn({0});
                bluetoothController.notifyNewDeltaTime(expectedDeltaTime);

                bluetoothController.update();

                Verify(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).Never();
            }

            SECTION("no client is connected")
            {
                When(Method(mockArduino, millis)).AlwaysReturn(1'001);
                When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).Return({0}).AlwaysReturn(emptyClientIds);
                When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu);
                bluetoothController.notifyNewDeltaTime(expectedDeltaTime);

                bluetoothController.update();

                Verify(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).Never();
            }

            SECTION("deltaTimes vector is empty")
            {
                When(Method(mockArduino, millis)).AlwaysReturn(1'001);
                When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu);
                When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn({0});

                bluetoothController.update();

                Verify(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).Never();
            }
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
}
// NOLINTEND(readability-magic-numbers)