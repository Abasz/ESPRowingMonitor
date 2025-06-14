// NOLINTBEGIN(readability-magic-numbers)
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_container_properties.hpp"
#include "fakeit.hpp"

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
#include "../../../src/utils/configuration.h"
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
    Mock<IConnectionManagerCallbacks> mockConnectionManagerCallbacks;

    const RowingDataModels::RowingMetrics expectedData{
        .distance = 101.121,
        .lastRevTime = 2'000'000ULL,
        .lastStrokeTime = 1'600'000ULL,
        .strokeCount = 10U,
        .driveDuration = 1'001'000U,
        .recoveryDuration = 1'003'000U,
        .avgStrokePower = 70.1212,
        .dragCoefficient = 110 / 1e6,
        .driveHandleForces = {1.1, 2.2, 100.1},
    };
    const unsigned int bleUpdateInterval = 1'000 + 1;
    const std::vector<unsigned char> emptyClientIds{};

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

    When(Method(mockBaseMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockBaseMetricsBleService, getClientIds)).AlwaysReturn({0});
    Fake(Method(mockBaseMetricsBleService, broadcastBaseMetrics));

    When(Method(mockExtendedMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());
    When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).AlwaysReturn({0});
    When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn(emptyClientIds);
    When(Method(mockExtendedMetricsBleService, getExtendedMetricsClientIds)).AlwaysReturn(emptyClientIds);
    Fake(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics));
    Fake(Method(mockExtendedMetricsBleService, broadcastHandleForces));
    When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(0);

    BluetoothController bluetoothController(mockEEPROMService.get(), mockOtaUpdaterService.get(), mockSettingsBleService.get(), mockBatteryBleService.get(), mockDeviceInfoBleService.get(), mockOtaBleService.get(), mockBaseMetricsBleService.get(), mockExtendedMetricsBleService.get(), mockConnectionManagerCallbacks.get());

    SECTION("notifyBattery method should")
    {
        const auto expectedBatteryLevel = 66;

        Fake(Method(mockBatteryBleService, setBatteryLevel));
        Fake(Method(mockBatteryBleService, broadcastBatteryLevel));

        SECTION("set new battery level")
        {
            bluetoothController.notifyBattery(expectedBatteryLevel);

            Verify(Method(mockBatteryBleService, setBatteryLevel).Using(expectedBatteryLevel));
        }

        SECTION("notify")
        {
            bluetoothController.notifyBattery(expectedBatteryLevel);

            Verify(Method(mockBatteryBleService, broadcastBatteryLevel));
        }
    }

    SECTION("notifyNewMetrics method should")
    {
        SECTION("when extended metrics is")
        {
            SECTION("not subscribed should not broadcast")
            {
                When(Method(mockExtendedMetricsBleService, getExtendedMetricsClientIds)).AlwaysReturn(emptyClientIds);

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics)).Never();
            }
            SECTION("subscribed should broadcast with the correct parameters")
            {
                When(Method(mockExtendedMetricsBleService, getExtendedMetricsClientIds)).AlwaysReturn({0});

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics).Using(Eq(expectedData.avgStrokePower), Eq(expectedData.recoveryDuration), Eq(expectedData.driveDuration), Eq(expectedData.dragCoefficient))).Once();
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
                    .lastRevTime = 2'000,
                    .lastStrokeTime = 1'600,
                    .strokeCount = 10,
                    .driveDuration = 1'001,
                    .recoveryDuration = 1'003,
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
                When(Method(mockBaseMetricsBleService, getClientIds)).Return(emptyClientIds);

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)).Never();
            }

            SECTION("subscribed should broadcast with the correct parameters")
            {
                const BleMetricsModel::BleMetricsData expectedBleData{
                    .revTime = expectedData.lastRevTime,
                    .distance = expectedData.distance,
                    .strokeTime = expectedData.lastStrokeTime,
                    .strokeCount = expectedData.strokeCount,
                    .avgStrokePower = expectedData.avgStrokePower,
                };
                When(Method(mockBaseMetricsBleService, getClientIds)).Return({0});

                bluetoothController.notifyNewMetrics(expectedData);

                Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)
                           .Matching([&expectedBleData](const BleMetricsModel::BleMetricsData &data)
                                     { return data.revTime == expectedBleData.revTime &&
                                              data.distance == expectedBleData.distance &&
                                              data.strokeTime == expectedBleData.strokeTime &&
                                              data.strokeCount == expectedBleData.strokeCount &&
                                              data.avgStrokePower == expectedBleData.avgStrokePower; }))
                    .Once();
            }
        }

        SECTION("reset lastMetricsBroadcastTime")
        {
            When(Method(mockArduino, millis)).Return(bleUpdateInterval, bleUpdateInterval * 2 - 1);
            When(Method(mockBaseMetricsBleService, getClientIds)).AlwaysReturn({0});

            bluetoothController.notifyNewMetrics(expectedData);
            mockBaseMetricsBleService.ClearInvocationHistory();
            bluetoothController.update();

            Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)).Never();
        }
    }

    SECTION("notifyNewDeltaTime method should")
    {
        std::vector<unsigned long> expectedDeltaTimes{10'000, 11'000, 12'000, 11'000};
        const auto expectedDeltaTime = 10'000UL;
        const auto minimumDeltaTimeMtu = 100;
        const std::vector<unsigned char> clientIds{0};

        Fake(Method(mockExtendedMetricsBleService, broadcastDeltaTimes));

        SECTION("ignore new value when no client is connected")
        {
            std::vector<std::vector<unsigned long>> resultDeltaTimes;

            When(Method(mockArduino, millis)).AlwaysReturn(bleUpdateInterval);
            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn(emptyClientIds);
            When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu);
            When(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).AlwaysDo([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                                      {
                            resultDeltaTimes.push_back(deltaTimes);

                            return true; });

            bluetoothController.notifyNewDeltaTime(expectedDeltaTime);
            bluetoothController.update();

            Verify(Method(mockExtendedMetricsBleService, calculateMtu)).Never();
            REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::IsEmpty());
        }

        SECTION("ignore new value when client MTU is below 100")
        {
            std::vector<std::vector<unsigned long>> resultDeltaTimes;

            When(Method(mockArduino, millis)).AlwaysReturn(bleUpdateInterval);
            When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu - 1);
            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn(clientIds);
            When(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).AlwaysDo([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                                      {
                            resultDeltaTimes.push_back(deltaTimes);

                            return true; });

            bluetoothController.notifyNewDeltaTime(expectedDeltaTime);
            bluetoothController.update();

            REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::IsEmpty());
        }

        SECTION("add new value to deltaTimes array when client MTU is at least 100")
        {
            std::vector<std::vector<unsigned long>> resultDeltaTimes;

            When(Method(mockArduino, millis)).AlwaysReturn(bleUpdateInterval + 1);
            When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu);
            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn(clientIds);
            When(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).AlwaysDo([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                                      {
                            resultDeltaTimes.push_back(deltaTimes);

                            return true; });

            bluetoothController.notifyNewDeltaTime(expectedDeltaTime);
            bluetoothController.update();

            REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::SizeIs(1));
        }

        SECTION("flush deltaTimes via bluetooth when client MTU capacity is reached")
        {
            std::vector<std::vector<unsigned long>> resultDeltaTimes;

            When(Method(mockArduino, millis)).AlwaysReturn(bleUpdateInterval);
            When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu);
            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn(clientIds);
            Fake(Method(mockExtendedMetricsBleService, broadcastDeltaTimes).Matching([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                                     {
                                    resultDeltaTimes.push_back(deltaTimes);

                                    return true; }));

            auto i = 0U;
            while ((i + 1) * sizeof(unsigned long) < minimumDeltaTimeMtu - 3)
            {
                bluetoothController.notifyNewDeltaTime(expectedDeltaTime + i);
                ++i;
            }

            REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::SizeIs(1));
            REQUIRE_THAT(resultDeltaTimes[0], Catch::Matchers::SizeIs(i));
        }

        SECTION("reset lastDeltaTimesBroadcastTime after flushing deltaTimes via bluetooth")
        {
            When(Method(mockArduino, millis)).Return(bleUpdateInterval, bleUpdateInterval, (bleUpdateInterval - 1) * 2);
            When(Method(mockExtendedMetricsBleService, getDeltaTimesClientIds)).AlwaysReturn(clientIds);
            When(Method(mockExtendedMetricsBleService, calculateMtu)).AlwaysReturn(minimumDeltaTimeMtu);

            bluetoothController.notifyNewDeltaTime(expectedDeltaTime);
            bluetoothController.update();
            bluetoothController.notifyNewDeltaTime(expectedDeltaTime);
            bluetoothController.update();

            Verify(Method(mockExtendedMetricsBleService, broadcastDeltaTimes)).Once();
        }
    }
}
// NOLINTEND(readability-magic-numbers)