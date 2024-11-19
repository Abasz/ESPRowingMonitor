// NOLINTBEGIN(readability-magic-numbers)
#include <vector>

#include "../include/catch_amalgamated.hpp"
#include "../include/fakeit.hpp"

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
    When(Method(mockExtendedMetricsBleService, setup)).AlwaysReturn(&mockNimBLEService.get());

    BluetoothController bluetoothController(mockEEPROMService.get(), mockOtaUpdaterService.get(), mockSettingsBleService.get(), mockBatteryBleService.get(), mockDeviceInfoBleService.get(), mockOtaBleService.get(), mockBaseMetricsBleService.get(), mockExtendedMetricsBleService.get());

    SECTION("notifyBattery method should")
    {
        const auto expectedBatteryLevel = 66;

        Fake(Method(mockBatteryBleService, setBatteryLevel));
        Fake(Method(mockBatteryBleService, broadcastBatteryLevel));

        SECTION("set new battery level")
        {
            Fake(Method(mockBatteryBleService, isSubscribed));

            bluetoothController.notifyBattery(expectedBatteryLevel);

            Verify(Method(mockBatteryBleService, setBatteryLevel).Using(expectedBatteryLevel));
        }

        SECTION("not notify if there are no subscribers")
        {
            When(Method(mockBatteryBleService, isSubscribed)).Return(0);

            bluetoothController.notifyBattery(expectedBatteryLevel);

            VerifyNoOtherInvocations(Method(mockBatteryBleService, broadcastBatteryLevel));
        }

        SECTION("notify if there are subscribers")
        {
            When(Method(mockBatteryBleService, isSubscribed)).Return(1);

            bluetoothController.notifyBattery(expectedBatteryLevel);

            Verify(Method(mockBatteryBleService, broadcastBatteryLevel));
        }
    }

    SECTION("notifyBaseMetrics method should")
    {
        const unsigned short revTime = 31'000U;
        const unsigned int revCount = 360'000U;
        const unsigned short strokeTime = 30'100U;
        const unsigned short strokeCount = 2'000U;
        const short avgStrokePower = 300;

        Fake(Method(mockBaseMetricsBleService, broadcastBaseMetrics));

        SECTION("not broadcast if there are no subscribers")
        {
            When(Method(mockBaseMetricsBleService, isSubscribed)).Return(0);

            bluetoothController.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics)).Never();
        }

        SECTION("broadcast new base metrics with the correct parameters")
        {
            When(Method(mockBaseMetricsBleService, isSubscribed)).Return(1);

            bluetoothController.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(Method(mockBaseMetricsBleService, broadcastBaseMetrics).Using(revTime, revCount, strokeTime, strokeCount, avgStrokePower)).Once();
        }
    }

    SECTION("notifyExtendedMetrics method should")
    {
        const unsigned int recoveryDuration = 4'000'000;
        const unsigned int driveDuration = 3'000'000;
        const unsigned char dragFactor = 110;
        const short avgStrokePower = 300;

        Fake(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics));

        SECTION("not broadcast if there are no subscribers")
        {
            When(Method(mockExtendedMetricsBleService, isExtendedMetricsSubscribed)).AlwaysReturn(false);

            bluetoothController.notifyExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragFactor);

            Verify(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics)).Never();
        }

        SECTION("broadcast ExtendedMetrics when there are subscribers")
        {
            When(Method(mockExtendedMetricsBleService, isExtendedMetricsSubscribed)).AlwaysReturn(true);

            bluetoothController.notifyExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragFactor);

            Verify(Method(mockExtendedMetricsBleService, broadcastExtendedMetrics).Using(avgStrokePower, recoveryDuration, driveDuration, dragFactor)).Once();
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

    SECTION("notifyHandleForces method should")
    {
        const std::vector<float> expectedHandleForces{1.1, 3.3, 500.4, 300.4};

        When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).Return({0});
        Fake(Method(mockExtendedMetricsBleService, broadcastHandleForces));

        SECTION("not notify if there are no subscribers")
        {
            When(Method(mockExtendedMetricsBleService, getHandleForcesClientIds)).Return({});

            bluetoothController.notifyHandleForces(expectedHandleForces);

            Verify(Method(mockExtendedMetricsBleService, broadcastHandleForces)).Never();
        }

        SECTION("not notify if handleForces vector is empty")
        {
            bluetoothController.notifyHandleForces({});

            Verify(Method(mockExtendedMetricsBleService, broadcastHandleForces)).Never();
        }

        SECTION("broadcast handleForces when vector is not empty and there are subscribers")
        {
            bluetoothController.notifyHandleForces(expectedHandleForces);

            Verify(Method(mockExtendedMetricsBleService, broadcastHandleForces).Using(expectedHandleForces)).Once();
        }
    }
}
// NOLINTEND(readability-magic-numbers)