// NOLINTBEGIN(readability-magic-numbers)
#include <array>
#include <string>

#include "./include/catch_amalgamated.hpp"
#include "./include/fakeit.hpp"

#include "./include/FastLED.h"

#include "../../src/peripherals/bluetooth/bluetooth.controller.interface.h"
#include "../../src/peripherals/peripherals.controller.h"
#include "../../src/peripherals/sd-card/sd-card.service.interface.h"
#include "../../src/utils/EEPROM/EEPROM.service.interface.h"

using namespace fakeit;

TEST_CASE("PeripheralController", "[peripheral]")
{
    Mock<IBluetoothController> mockBluetoothController;
    Mock<ISdCardService> mockSdCardService;
    Mock<IEEPROMService> mockEEPROMService;

    mockFastLED.Reset();
    mockArduino.Reset();

    const auto expectedDeltaTime = 10000;
    const auto minimumDeltaTimeMTU = 100;
    const auto blinkInterval = Configurations::ledBlinkFrequency + 1U;
    const auto batteryLevel = 90;

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

    When(Method(mockBluetoothController, getDeltaTimesMTU)).AlwaysReturn(23);
    When(Method(mockBluetoothController, isDeltaTimesSubscribed)).AlwaysReturn(false);
    When(Method(mockBluetoothController, isAnyDeviceConnected)).AlwaysReturn(false);
    Fake(Method(mockBluetoothController, notifyBaseMetrics));
    Fake(Method(mockBluetoothController, notifyDeltaTimes));
    Fake(Method(mockBluetoothController, notifyHandleForces));
    Fake(Method(mockBluetoothController, notifyExtendedMetrics));

    When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);
    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(false);
    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(false);
    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(false);
    Fake(Method(mockSdCardService, saveDeltaTime));

    Fake(Method(mockFastLED, show));
    Fake(Method(mockFastLED, mockHelperSetColor));

    PeripheralsController peripheralsController(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get());

    SECTION("begin method should")
    {
        Fake(Method(mockSdCardService, setup));

        Fake(Method(mockBluetoothController, setup));

        Fake(Method(mockFastLED, addLeds));

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

        SECTION("setup FastLED")
        {
            peripheralsController.begin();

            Verify(Method(mockFastLED, addLeds).Using(Ne(nullptr), Eq(1), Eq(0))).Once();
        }
    }

    SECTION("notifyBattery method should send notification of battery level")
    {
        const auto expectedBatteryLevel = 55U;
        Fake(Method(mockBluetoothController, notifyBattery));

        peripheralsController.notifyBattery(expectedBatteryLevel);

        Verify(Method(mockBluetoothController, notifyBattery).Using(Eq(expectedBatteryLevel))).Once();
    }

    SECTION("isAnyDeviceConnected method should return bluetooth connection status")
    {
        When(Method(mockBluetoothController, isAnyDeviceConnected)).Return(true, false);

        REQUIRE(peripheralsController.isAnyDeviceConnected() == true);
        REQUIRE(peripheralsController.isAnyDeviceConnected() == false);
    }

    SECTION("update method should")
    {
        const auto batteryLevel = 90;

        SECTION("update LED")
        {
            SECTION("in ledBlinkFrequency")
            {
                When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval, blinkInterval, blinkInterval * 2, blinkInterval * 2, blinkInterval * 2);

                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);
                peripheralsController.update(batteryLevel);

                Verify(Method(mockFastLED, show)).Exactly(2);
            }

            SECTION("to green when battery level is above 80")
            {
                PeripheralsController peripheralsControllerLedTest(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get());

                const auto maxBatteryLevel = 90;
                When(Method(mockArduino, millis)).AlwaysReturn(Configurations::ledBlinkFrequency + 1U);

                peripheralsControllerLedTest.update(maxBatteryLevel);

                Verify(Method(mockFastLED, mockHelperSetColor).Using(CRGB::Green)).Once();
            }

            SECTION("to red when battery level is below 30")
            {
                const auto minBatteryLevel = 29;
                When(Method(mockArduino, millis)).AlwaysReturn(Configurations::ledBlinkFrequency + 1U);
                PeripheralsController peripheralsControllerLedTest(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get());

                peripheralsControllerLedTest.update(minBatteryLevel);

                Verify(Method(mockFastLED, mockHelperSetColor).Using(CRGB::Red)).Once();
            }

            SECTION("to blue when battery level above 30 but below 80")
            {
                const auto normalBatteryLevel = 55;
                When(Method(mockArduino, millis)).AlwaysReturn(Configurations::ledBlinkFrequency + 1U);
                PeripheralsController peripheralsControllerLedTest(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get());

                peripheralsControllerLedTest.update(normalBatteryLevel);

                Verify(Method(mockFastLED, mockHelperSetColor).Using(CRGB::Blue)).Once();
            }

            SECTION("to black (i.e. off) when the last update turned the led on")
            {
                When(Method(mockArduino, millis)).Return(Configurations::ledBlinkFrequency + 1U, Configurations::ledBlinkFrequency + 1U, 0U, 0U, Configurations::ledBlinkFrequency + 1, Configurations::ledBlinkFrequency + 1U, 0U, Configurations::ledBlinkFrequency + 1U);
                PeripheralsController peripheralsControllerLedTest(mockBluetoothController.get(), mockSdCardService.get(), mockEEPROMService.get());

                peripheralsControllerLedTest.update(batteryLevel);
                peripheralsControllerLedTest.update(batteryLevel);

                Verify(Method(mockFastLED, mockHelperSetColor).Using(CRGB::Black)).Once();
            }
        }

        SECTION("notify with last available base metric when last notification was send more than 1 seconds ago")
        {
            const auto bleFlag = BleServiceFlag::CpsService;

            const auto secInMicroSec = 1e6L;
            const unsigned short bleRevTimeData = lroundl((expectedData.lastRevTime / secInMicroSec) * (bleFlag == BleServiceFlag::CpsService ? 2'048 : 1'024)) % USHRT_MAX;
            const unsigned int bleRevCountData = lround(expectedData.distance);
            const unsigned short bleStrokeTimeData = lroundl((expectedData.lastStrokeTime / secInMicroSec) * 1'024) % USHRT_MAX;
            const unsigned short bleStrokeCountData = expectedData.strokeCount;
            const short bleAvgStrokePowerData = static_cast<short>(lround(expectedData.avgStrokePower));

            When(Method(mockArduino, millis)).Return(0, 0, 1001);
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(bleFlag);
            peripheralsController.updateData(expectedData);

            peripheralsController.update(batteryLevel);

            Verify(Method(mockBluetoothController, notifyBaseMetrics).Using(Eq(bleRevTimeData), Eq(bleRevCountData), Eq(bleStrokeTimeData), Eq(bleStrokeCountData), Eq(bleAvgStrokePowerData))).Once();
        }

        SECTION("not notify base metric when last notification was send less than 1 seconds ago")
        {
            When(Method(mockArduino, millis)).Return(0, 999);

            peripheralsController.update(batteryLevel);

            Verify(Method(mockBluetoothController, notifyBaseMetrics)).Never();
        }

        SECTION("notify deltaTimes when last notification was send more than 1 seconds ago, clear vector and reserve memory based on MTU")
        {
            std::vector<std::vector<unsigned long>> notifiedDeltaTimes{};
            When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval, blinkInterval * 2, blinkInterval * 2);
            When(Method(mockBluetoothController, getDeltaTimesMTU)).AlwaysReturn(minimumDeltaTimeMTU);
            When(Method(mockBluetoothController, isDeltaTimesSubscribed)).AlwaysReturn(true);
            When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(true);
            Fake(Method(mockBluetoothController, notifyDeltaTimes).Matching([&notifiedDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                            {
                notifiedDeltaTimes.push_back(deltaTimes);

                return true; }));
            peripheralsController.updateDeltaTime(expectedDeltaTime);

            peripheralsController.update(batteryLevel);

            REQUIRE_THAT(notifiedDeltaTimes, Catch::Matchers::SizeIs(1));
            REQUIRE_THAT(notifiedDeltaTimes[0], Catch::Matchers::RangeEquals(std::vector<unsigned long>{expectedDeltaTime}));
            Verify(Method(mockBluetoothController, notifyDeltaTimes).Matching([](const std::vector<unsigned long> &deltaTimes)
                                                                              { return deltaTimes.capacity() == minimumDeltaTimeMTU / sizeof(unsigned long) + 1U && deltaTimes.empty(); }))
                .Once();
        }

        SECTION("not notify deltaTimes when last notification was send less than 1 seconds ago")
        {
            When(Method(mockArduino, millis)).Return(999);

            peripheralsController.update(batteryLevel);

            Verify(Method(mockBluetoothController, notifyBaseMetrics)).Never();
        }
    }

    SECTION("updateDeltaTime method should")
    {
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
            SECTION("skip adding new deltaTime when")
            {
                SECTION("logging to bluetooth is disabled")
                {
                    std::vector<std::vector<unsigned long>> resultDeltaTimes;

                    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(false);
                    When(Method(mockBluetoothController, isDeltaTimesSubscribed)).AlwaysReturn(true);
                    When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval);

                    Fake(Method(mockBluetoothController, notifyDeltaTimes).Matching([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                                    {
                            resultDeltaTimes.push_back(deltaTimes);

                            return true; }));

                    peripheralsController.updateDeltaTime(expectedDeltaTime);

                    peripheralsController.update(batteryLevel);
                    REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::SizeIs(1));
                    REQUIRE_THAT(resultDeltaTimes[0], Catch::Matchers::IsEmpty());
                }

                SECTION("no client has subscribed")
                {
                    std::vector<std::vector<unsigned long>> resultDeltaTimes;

                    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(true);
                    When(Method(mockBluetoothController, isDeltaTimesSubscribed)).AlwaysReturn(false);
                    When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval);

                    Fake(Method(mockBluetoothController, notifyDeltaTimes).Matching([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                                    {
                            resultDeltaTimes.push_back(deltaTimes);

                            return true; }));

                    peripheralsController.updateDeltaTime(expectedDeltaTime);

                    peripheralsController.update(batteryLevel);
                    REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::SizeIs(1));
                    REQUIRE_THAT(resultDeltaTimes[0], Catch::Matchers::IsEmpty());
                }

                SECTION("client MTU is below 100")
                {
                    std::vector<std::vector<unsigned long>> resultDeltaTimes;

                    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(true);
                    When(Method(mockBluetoothController, getDeltaTimesMTU)).AlwaysReturn(minimumDeltaTimeMTU - 1);
                    When(Method(mockBluetoothController, isDeltaTimesSubscribed)).AlwaysReturn(true);
                    When(Method(mockArduino, millis)).Return(blinkInterval, blinkInterval);

                    Fake(Method(mockBluetoothController, notifyDeltaTimes).Matching([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                                    {
                            resultDeltaTimes.push_back(deltaTimes);

                            return true; }));

                    peripheralsController.updateDeltaTime(expectedDeltaTime);

                    peripheralsController.update(batteryLevel);
                    REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::SizeIs(1));
                    REQUIRE_THAT(resultDeltaTimes[0], Catch::Matchers::IsEmpty());
                }
            }
        }

        SECTION("flush deltaTimes via bluetooth when client MTU capacity is reached")
        {
            std::vector<std::vector<unsigned long>> resultDeltaTimes;

            When(Method(mockArduino, millis)).AlwaysReturn(blinkInterval);
            When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(true);
            When(Method(mockBluetoothController, getDeltaTimesMTU)).AlwaysReturn(minimumDeltaTimeMTU);
            When(Method(mockBluetoothController, isDeltaTimesSubscribed)).AlwaysReturn(true);
            Fake(Method(mockBluetoothController, notifyDeltaTimes).Matching([&resultDeltaTimes](const std::vector<unsigned long> &deltaTimes)
                                                                            {
                            resultDeltaTimes.push_back(deltaTimes);

                            return true; }));

            auto i = 0U;
            while ((i + 1) * sizeof(unsigned long) < minimumDeltaTimeMTU - 3)
            {
                peripheralsController.updateDeltaTime(expectedDeltaTime + i);
                i++;
            }

            REQUIRE_THAT(resultDeltaTimes, Catch::Matchers::SizeIs(1));
            REQUIRE_THAT(resultDeltaTimes[0], Catch::Matchers::SizeIs(i));
        }

        SECTION("reset lastDeltaTimesBroadcastTime after flushing deltaTimes via bluetooth")
        {
            const unsigned int bleUpdateInterval = 1'000;

            When(Method(mockArduino, millis)).Return(bleUpdateInterval, bleUpdateInterval * 2 - 1).AlwaysReturn(bleUpdateInterval * 3 - 1);
            When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(true);
            When(Method(mockBluetoothController, getDeltaTimesMTU)).AlwaysReturn(minimumDeltaTimeMTU);
            When(Method(mockBluetoothController, isDeltaTimesSubscribed)).AlwaysReturn(true);

            auto i = 0U;
            while ((i + 1) * sizeof(unsigned long) < minimumDeltaTimeMTU - 3)
            {
                peripheralsController.updateDeltaTime(expectedDeltaTime + i);
                i++;
            }

            peripheralsController.update(batteryLevel);

            Verify(Method(mockBluetoothController, notifyDeltaTimes)).Once();
        }
    }

    SECTION("updateData method should")
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

                peripheralsController.updateData(expectedData);

                Verify(Method(mockBluetoothController, notifyBaseMetrics).Using(Eq(bleRevTimeDataCsc), Eq(bleRevCountData), Eq(bleStrokeTimeData), Eq(bleStrokeCountData), Eq(bleAvgStrokePowerData))).Once();
            }

            SECTION("when bleServiceFlag is PSC")
            {
                const unsigned short bleRevTimeDataPcs = lroundl((expectedData.lastRevTime / secInMicroSec) * 2'048) % USHRT_MAX;

                When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);

                peripheralsController.updateData(expectedData);

                Verify(Method(mockBluetoothController, notifyBaseMetrics).Using(Eq(bleRevTimeDataPcs), Eq(bleRevCountData), Eq(bleStrokeTimeData), Eq(bleStrokeCountData), Eq(bleAvgStrokePowerData))).Once();
            }
        }

        SECTION("notify extended metrics")
        {
            peripheralsController.updateData(expectedData);

            Verify(Method(mockBluetoothController, notifyExtendedMetrics).Using(Eq(bleAvgStrokePowerData), Eq(expectedData.recoveryDuration), Eq(expectedData.driveDuration), Eq(lround(expectedData.dragCoefficient * 1e6)))).Once();
        }

        SECTION("notify handleForces")
        {
            peripheralsController.updateData(expectedData);

            Verify(Method(mockBluetoothController, notifyHandleForces).Using(Eq(expectedData.driveHandleForces))).Once();
        }

        SECTION("reset lastMetricsBroadcastTime")
        {
            const unsigned int bleUpdateInterval = 1'000;

            When(Method(mockArduino, millis)).Return(bleUpdateInterval, bleUpdateInterval * 2 - 1).AlwaysReturn(bleUpdateInterval * 3 - 1);

            peripheralsController.updateData(expectedData);
            peripheralsController.update(batteryLevel);

            Verify(Method(mockBluetoothController, notifyBaseMetrics)).Once();
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
                                                                     { return deltaTimes.capacity() == (Configurations::minimumRecoveryTime + Configurations::minimumDriveTime) / Configurations::rotationDebounceTimeMin && deltaTimes.empty(); }));
        }
    }
}
// NOLINTEND(readability-magic-numbers)