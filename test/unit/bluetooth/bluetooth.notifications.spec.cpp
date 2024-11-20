// NOLINTBEGIN(readability-magic-numbers)
#include <vector>

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
#include "../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../src/utils/enums.h"
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("BluetoothController", "[callbacks]")
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

    When(Method(mockArduino, millis)).AlwaysReturn(0);

    Fake(Method(mockNimBLEServer, createServer));
    Fake(Method(mockNimBLEServer, init));
    Fake(Method(mockNimBLEServer, setPower));
    Fake(Method(mockNimBLEServer, start));

    Fake(Method(mockNimBLEService, start));

    Fake(Method(mockNimBLEAdvertising, start));
    Fake(Method(mockNimBLEAdvertising, setAppearance));
    Fake(Method(mockNimBLEAdvertising, addServiceUUID));

    When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);

    Fake(Method(mockOtaUpdaterService, begin));

    When(Method(mockSettingsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockBatteryBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockDeviceInfoBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockOtaBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockOtaBleService, getOtaTx)).AlwaysReturn(&mockNimBLECharacteristic.get());
    When(Method(mockExtendedMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());

    When(Method(mockBaseMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockBaseMetricsBleService, isSubscribed)).AlwaysReturn(true);
    Fake(Method(mockBaseMetricsBleService, broadcastBaseMetrics));

    When(Method(mockExtendedMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).AlwaysReturn({0});
    When(Method(mockExtendedMetricsBleService, isExtendedMetricsSubscribed)).AlwaysReturn(true);
    Fake(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics));
    Fake(Method(mockExtendedMetricsBleService, broadcastHandleForces));

    BluetoothController bluetoothController(mockEEPROMService.get(), mockOtaUpdaterService.get(), mockSettingsBleService.get(), mockBatteryBleService.get(), mockDeviceInfoBleService.get(), mockOtaBleService.get(), mockBaseMetricsBleService.get(), mockExtendedMetricsBleService.get());

    SECTION("notifyBattery method should")
    {
        const auto expectedBatteryLevel = 66;

        Fake(Method(mockBatteryBleService, setBatteryLevel));
        Fake(Method(mockBatteryBleService, broadcastBatteryLevel));

        SECTION("set new battery level")
        {
            When(Method(mockBatteryBleService, isSubscribed)).Return(true);

            bluetoothController.notifyBattery(expectedBatteryLevel);

            Verify(Method(mockBatteryBleService, setBatteryLevel).Using(expectedBatteryLevel));
        }

        SECTION("not notify if there are no subscribers")
        {
            When(Method(mockBatteryBleService, isSubscribed)).Return(false);

            bluetoothController.notifyBattery(expectedBatteryLevel);

            VerifyNoOtherInvocations(Method(mockBatteryBleService, broadcastBatteryLevel));
        }

        SECTION("notify if there are subscribers")
        {
            When(Method(mockBatteryBleService, isSubscribed)).Return(true);

            bluetoothController.notifyBattery(expectedBatteryLevel);

            Verify(Method(mockBatteryBleService, broadcastBatteryLevel));
        }
    }

    SECTION("notifyNewMetrics method should")
    {
        const short bleAvgStrokePowerData = static_cast<short>(lround(expectedData.avgStrokePower));

        SECTION("convert metrics to BLE compliant format")
        {
            const auto secInMicroSec = 1e6L;
            const unsigned int bleRevCountData = lround(expectedData.distance);
            const unsigned short bleStrokeTimeData = lroundl((expectedData.lastStrokeTime / secInMicroSec) * 1'024) % USHRT_MAX;
            const unsigned short bleStrokeCountData = expectedData.strokeCount;

            SECTION("when bleServiceFlag is CSC")
            {
                const unsigned short bleRevTimeDataCsc = lroundl((expectedData.lastRevTime / secInMicroSec) * 1'024) % USHRT_MAX;

                When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics).Using(Eq(bleRevTimeDataCsc), Eq(bleRevCountData), Eq(bleStrokeTimeData), Eq(bleStrokeCountData), Eq(bleAvgStrokePowerData))).Once();
            }

            SECTION("when bleServiceFlag is PSC")
            {
                const unsigned short bleRevTimeDataPcs = lroundl((expectedData.lastRevTime / secInMicroSec) * 2'048) % USHRT_MAX;

                When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics).Using(Eq(bleRevTimeDataPcs), Eq(bleRevCountData), Eq(bleStrokeTimeData), Eq(bleStrokeCountData), Eq(bleAvgStrokePowerData))).Once();
            }
        }

        SECTION("when extended metrics is")
        {
            SECTION("not subscribed should not broadcast")
            {
                When(Method(mockExtendedMetricsBleService, isExtendedMetricsSubscribed)).Return(false);

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics)).Never();
            }
            SECTION("subscribed should broadcast with the correct parameters")
            {
                When(Method(mockExtendedMetricsBleService, isExtendedMetricsSubscribed)).Return(true);

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics).Using(Eq(bleAvgStrokePowerData), Eq(expectedData.recoveryDuration), Eq(expectedData.driveDuration), Eq(lround(expectedData.dragCoefficient * 1e6)))).Once();
            }
        }

        SECTION("when notify handleForces is")
        {
            SECTION("not subscribed should not broadcast")
            {
                When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).Return({});

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockExtendedMetricsBleService, broadcastHandleForces)).Never();
            }

            SECTION("empty should not broadcast")
            {
                const RowingDataModels::RowingMetrics driveForcesEmptyData{
                    .distance = 100,
                    .lastRevTime = 2000,
                    .lastStrokeTime = 1600,
                    .strokeCount = 10,
                    .driveDuration = 1001,
                    .recoveryDuration = 1003,
                    .avgStrokePower = 70,
                    .dragCoefficient = 0.00001,
                    .driveHandleForces = {},
                };

                When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).Return({0});

                bluetoothController.notifyNewMetrics(driveForcesEmptyData);

                Verify(Method(mockExtendedMetricsBleService, broadcastHandleForces)).Never();
            }

            SECTION("subscribed and not empty should broadcast with the correct parameters")
            {
                When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).Return({0});

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockExtendedMetricsBleService, broadcastHandleForces).Using(Eq(expectedData.driveHandleForces))).Once();
            }
        }

        SECTION("when base metrics is")
        {
            Fake(Method(mockBaseMetricsBleService, broadcastBaseMetrics));

            SECTION("not subscribed should not subscribers")
            {
                When(Method(mockBaseMetricsBleService, isSubscribed)).Return(false);

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)).Never();
            }

            SECTION("subscribed should broadcast with the correct parameters")
            {
                const auto secInMicroSec = 1e6L;
                const unsigned int bleRevCountData = lround(expectedData.distance);
                const unsigned short bleRevTimeDataPcs = lroundl((expectedData.lastRevTime / secInMicroSec) * 2'048) % USHRT_MAX;
                const unsigned short bleStrokeTimeData = lroundl((expectedData.lastStrokeTime / secInMicroSec) * 1'024) % USHRT_MAX;
                const unsigned short bleStrokeCountData = expectedData.strokeCount;

                When(Method(mockBaseMetricsBleService, isSubscribed)).Return(true);

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics).Using(bleRevTimeDataPcs, bleRevCountData, bleStrokeTimeData, bleStrokeCountData, bleAvgStrokePowerData)).Once();
            }
        }

        SECTION("reset lastMetricsBroadcastTime")
        {
            const unsigned int bleUpdateInterval = 1'000;

            When(Method(mockArduino, millis)).Return(bleUpdateInterval, bleUpdateInterval * 2 - 1);
            When(Method(mockBaseMetricsBleService, isSubscribed)).AlwaysReturn(true);

            bluetoothController.notifyNewMetrics(expectedData);
            mockBaseMetricsBleService.ClearInvocationHistory();
            bluetoothController.update();

            Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)).Never();
        }
    }

    SECTION("notifyDeltaTimes method should")
    {
        std::vector<unsigned long> expectedDeltaTimes{10000, 11000, 12000, 11000};

        When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).Return({0});
        Fake(Method(mockExtendedMetricsBleService, broadcastDeltaTimes));

        SECTION("not notify if there are no subscribers")
        {
            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).Return({});

            bluetoothController.notifyDeltaTimes(expectedDeltaTimes);

            Verify(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).Never();
        }

        SECTION("not notify if deltaTimes vector is empty")
        {
            bluetoothController.notifyDeltaTimes({});

            Verify(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).Never();
        }

        SECTION("broadcast deltaTimes when vector is not empty and there are subscribers")
        {
            bluetoothController.notifyDeltaTimes(expectedDeltaTimes);

            Verify(Method(mockExtendedMetricsBleService, broadcastDeltaTimes).Using(expectedDeltaTimes)).Once();
        }
    }
}

// NOLINTEND(readability-magic-numbers)