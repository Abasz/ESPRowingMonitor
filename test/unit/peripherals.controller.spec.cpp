// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_container_properties.hpp"
#include "catch2/matchers/catch_matchers_range_equals.hpp"
#include "fakeit.hpp"

#include "../../src/peripherals/bluetooth/bluetooth.controller.interface.h"
#include "../../src/peripherals/led/led.service.interface.h"
#include "../../src/peripherals/peripherals.controller.h"
#include "../../src/peripherals/sd-card/sd-card.service.interface.h"
#include "../../src/rower/stroke.model.h"
#include "../../src/utils/EEPROM/EEPROM.service.interface.h"

using namespace fakeit;

TEST_CASE("PeripheralController", "[peripheral]")
{
    mockArduino.Reset();

    Mock<IBluetoothController> mockBluetoothController;
    Mock<ISdCardService> mockSdCardService;
    Mock<IEEPROMService> mockEEPROMService;
    Mock<ILedService> mockLedService;

    const auto expectedDeltaTime = 10000UL;
    const auto blinkInterval = Configurations::ledBlinkFrequency + 1U;

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

    When(Method(mockBluetoothController, isAnyDeviceConnected)).AlwaysReturn(false);
    Fake(Method(mockBluetoothController, notifyNewDeltaTime));
    Fake(Method(mockBluetoothController, notifyNewMetrics));
    Fake(Method(mockBluetoothController, update));

    When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);
    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(false);
    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(false);
    When(Method(mockEEPROMService, getSensorSignalSettings)).AlwaysReturn(RowerProfile::SensorSignalSettings{});
    When(Method(mockEEPROMService, getStrokePhaseDetectionSettings)).AlwaysReturn(RowerProfile::StrokePhaseDetectionSettings{});
    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(false);
    Fake(Method(mockSdCardService, saveDeltaTime));

    Fake(Method(mockLedService, setColor));
    When(Method(mockLedService, getColor)).AlwaysReturn(LedColor::Black);
    Fake(Method(mockLedService, refresh));
    Fake(Method(mockLedService, clear));

    SECTION("begin method should")
    {
        PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());

        Fake(Method(mockSdCardService, setup));

        Fake(Method(mockBluetoothController, setup));

#if ENABLE_RUNTIME_SETTINGS
        SECTION("load runtime settings")
        {
            peripheralsController.begin();

            Verify(Method(mockEEPROMService, getSensorSignalSettings)).Once();
        }
#else
        SECTION("not load runtime settings")
        {
            peripheralsController.begin();

            Verify(Method(mockEEPROMService, getSensorSignalSettings)).Never();
        }
#endif

        SECTION("setup SdCardService")
        {
            peripheralsController.begin();

            Verify(Method(mockSdCardService, setup)).Once();
        }

        SECTION("setup BluetoothController")
        {
            peripheralsController.begin();

            Verify(Method(mockBluetoothController, setup)).Once();
        }
    }

    SECTION("notifyBattery method should send notification of battery level")
    {
        const auto expectedBatteryLevel = 55U;
        PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
        Fake(Method(mockBluetoothController, notifyBattery));

        peripheralsController.notifyBattery(expectedBatteryLevel);

        Verify(Method(mockBluetoothController, notifyBattery).Using(Eq(expectedBatteryLevel))).Once();
    }

    SECTION("isAnyDeviceConnected method should return bluetooth connection status")
    {
        PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
        When(Method(mockBluetoothController, isAnyDeviceConnected)).Return(true, false);

        REQUIRE(peripheralsController.isAnyDeviceConnected() == true);
        REQUIRE(peripheralsController.isAnyDeviceConnected() == false);
    }

    SECTION("update method should")
    {
        const auto batteryLevel = 90;

        SECTION("when a device connected")
        {
            SECTION("not update LED when its already on with same color")
            {
                PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
                When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval * 2);
                When(Method(mockBluetoothController, isAnyDeviceConnected)).Return(false).AlwaysReturn(true);
                When(Method(mockLedService, getColor)).AlwaysReturn(LedColor::Green);
                // Set the led state to on
                peripheralsController.update(batteryLevel);
                mockLedService.ClearInvocationHistory();

                peripheralsController.update(batteryLevel);

                Verify(Method(mockLedService, refresh)).Never();
            }

            SECTION("update LED when color should be changed")
            {
                const auto minBatteryLevel = 29;
                PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
                When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval * 2);
                When(Method(mockBluetoothController, isAnyDeviceConnected)).Return(false).AlwaysReturn(true);
                When(Method(mockLedService, getColor)).AlwaysReturn(LedColor::Green);
                // Set the led state to on
                peripheralsController.update(batteryLevel);
                mockLedService.ClearInvocationHistory();

                peripheralsController.update(minBatteryLevel);

                Verify(Method(mockLedService, refresh)).Once();
            }

            SECTION("update LED when current color is black (i.e. off)")
            {
                PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
                When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval * 2);
                When(Method(mockBluetoothController, isAnyDeviceConnected)).AlwaysReturn(true);

                peripheralsController.update(batteryLevel);

                Verify(Method(mockLedService, refresh)).Once();
            }
        }

        SECTION("update LED")
        {
            SECTION("in ledBlinkFrequency")
            {
                PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
                When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval, blinkInterval, blinkInterval * 2, blinkInterval * 2, blinkInterval * 2);

                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);

                Verify(Method(mockLedService, refresh)).Exactly(2);
            }

            SECTION("to green when battery level is above 80")
            {
                const auto maxBatteryLevel = 90;
                PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
                When(Method(mockArduino, millis)).Return(blinkInterval);

                peripheralsController.update(maxBatteryLevel);

                Verify(Method(mockLedService, setColor).Using(LedColor::Green)).Once();
            }

            SECTION("to red when battery level is below 30")
            {
                const auto minBatteryLevel = 29;
                PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
                When(Method(mockArduino, millis)).Return(blinkInterval);

                peripheralsController.update(minBatteryLevel);

                Verify(Method(mockLedService, setColor).Using(LedColor::Red)).Once();
            }

            SECTION("to blue when battery level above 30 but below 80")
            {
                const auto normalBatteryLevel = 55;
                PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
                When(Method(mockArduino, millis)).Return(blinkInterval);

                peripheralsController.update(normalBatteryLevel);

                Verify(Method(mockLedService, setColor).Using(LedColor::Blue)).Once();
            }

            SECTION("to black (i.e. off) when the last update turned the led on")
            {
                PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
                When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval * 2, blinkInterval * 3, blinkInterval * 4);
                // Simulate alternating colors: Black -> Green -> Black -> Green -> Black
                When(Method(mockLedService, getColor)).Return(LedColor::Black, LedColor::Green, LedColor::Black, LedColor::Green);

                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);

                Verify(Method(mockLedService, setColor).Using(LedColor::Black)).Twice();
            }
        }

        SECTION("call update on BluetoothController")
        {
            PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());
            Fake(Method(mockBluetoothController, update));

            peripheralsController.update(batteryLevel);

            Verify(Method(mockBluetoothController, update)).Once();
        }
    }

    SECTION("updateDeltaTime method should")
    {
        PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());

        SECTION("to the sdCard data")
        {
            SECTION("add new deltaTime when logging to sd-card is enabled and log file is open")
            {
                std::vector<std::vector<unsigned long>> notifiedDeltaTimes{};

                When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(true);
                Fake(Method(mockSdCardService, saveDeltaTime).Matching([&notifiedDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                       {
                notifiedDeltaTimes.push_back(deltaTimes);

                return true; }));
                When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(true);
                When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval);

                peripheralsController.updateDeltaTime(expectedDeltaTime);

                peripheralsController.updateData({});
                REQUIRE_THAT(notifiedDeltaTimes, Catch::Matchers::SizeIs(1));
                REQUIRE_THAT(notifiedDeltaTimes[0], Catch::Matchers::RangeEquals(std::vector<unsigned long>{expectedDeltaTime}));
            }

            SECTION("skip adding new deltaTime when")
            {
                SECTION("logging to sd-card is disabled")
                {
                    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(true);
                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(false);
                    When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval);

                    peripheralsController.updateDeltaTime(expectedDeltaTime);

                    peripheralsController.updateData({});
                    Verify(Method(mockSdCardService, saveDeltaTime)).Never();
                }
                SECTION("log file is not open")
                {
                    std::vector<std::vector<unsigned long>> resultDeltaTimes;
                    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(false);
                    Fake(Method(mockSdCardService, saveDeltaTime).Matching([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                           {
                            resultDeltaTimes.push_back(deltaTimes);

                            return true; }));
                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(true);
                    When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval);

                    peripheralsController.updateDeltaTime(expectedDeltaTime);

                    peripheralsController.updateData({});
                    REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::SizeIs(1));
                    REQUIRE_THAT(resultDeltaTimes[0], Catch::Matchers::IsEmpty());
                }
            }
        }

        SECTION("to the bluetooth data")
        {
            SECTION("skip adding new deltaTime when logging to bluetooth is disabled")
            {
                When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(false);

                peripheralsController.updateDeltaTime(expectedDeltaTime);

                Verify(Method(mockBluetoothController, notifyNewDeltaTime)).Never();
            }

            SECTION("add new deltaTime when logging to bluetooth is enabled")
            {
                When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(true);

                peripheralsController.updateDeltaTime(expectedDeltaTime);

                Verify(Method(mockBluetoothController, notifyNewDeltaTime).Using(expectedDeltaTime)).Once();
            }
        }
    }

    SECTION("updateData method should")
    {
        PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get(), mockLedService.get());

        SECTION("notify new data to bluetooth controller")
        {
            peripheralsController.updateData(expectedData);

            Verify(Method(mockBluetoothController, notifyNewMetrics).Matching([&expectedData](const RowingDataModels::RowingMetrics &data)
                                                                              { return expectedData.avgStrokePower == data.avgStrokePower && expectedData.distance == data.distance && expectedData.dragCoefficient == data.dragCoefficient && expectedData.driveDuration == data.driveDuration && expectedData.driveHandleForces == data.driveHandleForces && expectedData.lastRevTime == data.lastRevTime && expectedData.lastStrokeTime == data.lastStrokeTime && expectedData.recoveryDuration == data.recoveryDuration && expectedData.strokeCount == data.strokeCount; }))
                .Once();
        }

        SECTION("when logToSdCard")
        {
            SECTION("is disabled not save delta times to sd-card")
            {
                When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(false);

                peripheralsController.updateData(expectedData);

                Verify(Method(mockSdCardService, saveDeltaTime)).Never();
            }
            SECTION("is enabled save delta times to sd-card")
            {
                When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(true);

                peripheralsController.updateData(expectedData);

                Verify(Method(mockSdCardService, saveDeltaTime)).Once();
            }
        }

        SECTION("reset sdDeltaTimes vector")
        {
            std::vector<std::vector<unsigned long>> savedDeltaTimes;

            When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(true);
            When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(true);
            Fake(Method(mockSdCardService, saveDeltaTime).Matching([&savedDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                   {
                savedDeltaTimes.push_back(deltaTimes);

                return true; }));

            peripheralsController.updateDeltaTime(expectedDeltaTime);
            peripheralsController.updateData(expectedData);

            REQUIRE_THAT(savedDeltaTimes, Catch::Matchers::SizeIs(1));
            REQUIRE_THAT(savedDeltaTimes[0], Catch::Matchers::RangeEquals(std::vector<unsigned long>{expectedDeltaTime}));
            Verify(Method(mockSdCardService, saveDeltaTime).Matching([](const std::vector<unsigned long> &deltaTimes)
                                                                     { return deltaTimes.capacity() == (RowerProfile::Defaults::minimumRecoveryTime + RowerProfile::Defaults::minimumDriveTime) / RowerProfile::Defaults::rotationDebounceTimeMin && deltaTimes.empty(); }));
        }
    }
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)