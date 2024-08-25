// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
#include "./include/catch_amalgamated.hpp"
#include "./include/fakeit.hpp"

#include "./include/Arduino.h"

#include "../../src/utils/configuration.h"
#include "../../src/utils/power-manager.controller.h"
#include "../../src/utils/power-manager.service.interface.h"

using namespace fakeit;

TEST_CASE("PowerManagerController", "[utils]")
{
    const unsigned long deepSleepTimeout = Configurations::deepSleepTimeout * 1'000UL;
    const unsigned long batteryMeasurementFrequency = Configurations::batteryMeasurementFrequency * 1'000UL;

    SECTION("begin method should initialize battery service and update battery level")
    {
        const unsigned char initialBatteryLevel = 75;

        Mock<IPowerManagerService> mockPowerManagerService;
        PowerManagerController powerManagerController(mockPowerManagerService.get());
        When(Method(mockPowerManagerService, setup)).AlwaysReturn(initialBatteryLevel);

        powerManagerController.begin();

        Verify(Method(mockPowerManagerService, setup)).Once();
        REQUIRE(powerManagerController.getBatteryLevel() == initialBatteryLevel);
    }

    SECTION("update method should")
    {
        const unsigned long lastRevTime = 1'000'000;
        const unsigned char newBatteryLevel = 50;

        Mock<IPowerManagerService> mockPowerManagerService;
        PowerManagerController powerManagerController(mockPowerManagerService.get());

        Fake(Method(mockPowerManagerService, goToSleep));
        When(Method(mockPowerManagerService, measureBattery)).AlwaysReturn(newBatteryLevel);

        SECTION("put the device to sleep if not connected and timeout exceeded")
        {
            const auto now = lastRevTime + deepSleepTimeout + 1;

            mockArduino.Reset();
            When(Method(mockArduino, micros)).AlwaysReturn(now);

            powerManagerController.update(lastRevTime, false);

            Verify(Method(mockPowerManagerService, goToSleep)).Once();
        }

        SECTION("not put the device to sleep if device is connected")
        {
            const auto now = lastRevTime + deepSleepTimeout + 1;

            mockArduino.Reset();
            When(Method(mockArduino, micros)).AlwaysReturn(now);

            powerManagerController.update(lastRevTime, true);

            Verify(Method(mockPowerManagerService, goToSleep)).Never();
        }

        SECTION("not put the device to sleep if timeout not exceeded regardless of connected device")
        {
            const auto now = lastRevTime + deepSleepTimeout - 1;

            mockArduino.Reset();
            When(Method(mockArduino, micros)).AlwaysReturn(now);

            powerManagerController.update(lastRevTime, false);

            Verify(Method(mockPowerManagerService, goToSleep)).Never();
        }

        SECTION("measure battery level at configured frequency")
        {
            const auto now = batteryMeasurementFrequency + 1;

            mockArduino.Reset();
            When(Method(mockArduino, micros)).AlwaysReturn(now);

            powerManagerController.update(0, true);

            Verify(Method(mockPowerManagerService, measureBattery)).Once();
            REQUIRE(powerManagerController.getBatteryLevel() == newBatteryLevel);
        }

        SECTION("not measure battery level if frequency not exceeded")
        {
            const auto now = batteryMeasurementFrequency - 1;

            mockArduino.Reset();
            When(Method(mockArduino, micros)).AlwaysReturn(now);

            powerManagerController.update(0, true);

            Verify(Method(mockPowerManagerService, measureBattery)).Never();
        }
    }

    SECTION("getBatteryLevel method should return the current battery level")
    {
        const unsigned char batteryLevel = 75;
        const auto now = batteryMeasurementFrequency + 1;

        mockArduino.Reset();
        Mock<IPowerManagerService> mockPowerManagerService;
        PowerManagerController powerManagerController(mockPowerManagerService.get());
        Fake(Method(mockPowerManagerService, goToSleep));
        When(Method(mockPowerManagerService, measureBattery)).AlwaysReturn(batteryLevel);
        When(Method(mockArduino, micros)).AlwaysReturn(now);

        powerManagerController.update(0, true);

        REQUIRE(powerManagerController.getBatteryLevel() == batteryLevel);
    }

    SECTION("setPreviousBatteryLevel method should update and getPreviousBatteryLevel method should return the previous battery level")
    {
        const unsigned char batteryLevelInitial = 75;
        const unsigned char batteryLevelNew = 70;
        const auto now = batteryMeasurementFrequency + 1;

        mockArduino.Reset();
        Mock<IPowerManagerService> mockPowerManagerService;
        PowerManagerController powerManagerController(mockPowerManagerService.get());
        Fake(Method(mockPowerManagerService, goToSleep));
        When(Method(mockPowerManagerService, measureBattery)).Return(batteryLevelInitial, batteryLevelNew);
        When(Method(mockArduino, micros)).Return(now, now * 2);

        powerManagerController.update(0, true);
        powerManagerController.setPreviousBatteryLevel();
        powerManagerController.update(0, true);

        REQUIRE(powerManagerController.getBatteryLevel() == batteryLevelNew);
        REQUIRE(powerManagerController.getPreviousBatteryLevel() == batteryLevelInitial);
    }
}
// NOLINTEND(cppcoreguidelines-avoid-do-while)