// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
#include "./include/catch_amalgamated.hpp"
#include "./include/fakeit.hpp"

#include "./include/Arduino.h"

#include "./include/globals.h"

#include "../../src/utils/configuration.h"
#include "../../src/utils/power-manager/power-manager.service.h"

using namespace fakeit;

TEST_CASE("PowerManagerService", "[utils]")
{
    PowerManagerService powerManager;
    const auto analogVoltage = static_cast<unsigned int>(round((Configurations::batteryVoltageMax - 0.1) * 1000));

    mockArduino.Reset();
    When(Method(mockArduino, analogReadMilliVolts)).AlwaysReturn(analogVoltage);
    When(Method(mockArduino, esp_sleep_get_wakeup_cause)).AlwaysReturn(ESP_SLEEP_WAKEUP_UNDEFINED);
    Fake(Method(mockArduino, pinMode));
    Fake(Method(mockArduino, digitalWrite));
    Fake(Method(mockArduino, delay));

    const auto batteryLevel = powerManager.setup();

    SECTION("setup method should")
    {
        SECTION("print wakeup reason")
        {
            Verify(Method(mockArduino, esp_sleep_get_wakeup_cause)).Once();
        }

        SECTION("initialize sensor")
        {
            Verify(Method(mockArduino, pinMode).Using(Configurations::sensorOnSwitchPinNumber, OUTPUT)).Once();
            Verify(Method(mockArduino, digitalWrite).Using(Configurations::sensorOnSwitchPinNumber, HIGH)).Once();
        }

        SECTION("initialize battery pin")
        {
            Verify(Method(mockArduino, pinMode).Using(Configurations::batteryPinNumber, INPUT)).Once();
        }

        SECTION("perform initial battery measurement")
        {
            Verify(Method(mockArduino, analogReadMilliVolts).Using(Configurations::batteryPinNumber)).Exactly(Configurations::initialBatteryLevelMeasurementCount * Configurations::batteryLevelArrayLength);

            const unsigned char expectedBatteryLevel = lround((analogVoltage / 1'000.0 - Configurations::batteryVoltageMin) / (Configurations::batteryVoltageMax - Configurations::batteryVoltageMin) * 100);

            REQUIRE(batteryLevel == expectedBatteryLevel);
        }
    }

    SECTION("goToSleep method should")
    {
        const auto wakeupPinState = HIGH;

        mockArduino.Reset();
        mockSerial.Reset();
        mockFastLED.Reset();
        When(Method(mockArduino, digitalRead)).Return(wakeupPinState);
        Fake(Method(mockArduino, pinMode));
        Fake(Method(mockArduino, digitalWrite));
        Fake(Method(mockArduino, esp_sleep_enable_ext0_wakeup));
        Fake(Method(mockArduino, esp_deep_sleep_start));
        Fake(Method(mockSerial, flush));
        Fake(Method(mockFastLED, clear));

        powerManager.goToSleep();

        SECTION("configure deep sleep mode and set wakeup pin")
        {
            Verify(Method(mockArduino, pinMode).Using(Configurations::wakeupPinNumber, INPUT_PULLUP)).Once();
            Verify(Method(mockArduino, digitalWrite).Using(Configurations::sensorOnSwitchPinNumber, LOW)).Once();
            Verify(Method(mockArduino, digitalRead).Using(Configurations::wakeupPinNumber)).Once();
            Verify(Method(mockArduino, esp_sleep_enable_ext0_wakeup).Using(Configurations::wakeupPinNumber, !wakeupPinState)).Once();
        }

        SECTION("start deep sleep mode")
        {
            Verify(Method(mockFastLED, clear).Using(true)).Once();
            Verify(Method(mockSerial, flush)).Once();
            Verify(Method(mockArduino, esp_deep_sleep_start)).Once();
        }
    }

    SECTION("measureBattery method should")
    {
        const auto analogVoltageMax = lround((Configurations::batteryVoltageMax + 1) * 1000);
        const auto analogVoltageMin = lround((Configurations::batteryVoltageMin - 1) * 1000);

        SECTION("return a value between 0 and 100")
        {
            mockArduino.Reset();
            When(Method(mockArduino, analogReadMilliVolts)).AlwaysReturn(analogVoltageMax);
            Fake(Method(mockArduino, delay));

            const auto batteryLevelMax = powerManager.measureBattery();

            REQUIRE(batteryLevelMax == 100);

            mockArduino.Reset();
            When(Method(mockArduino, analogReadMilliVolts)).AlwaysReturn(analogVoltageMin);
            Fake(Method(mockArduino, delay));

            const auto batteryLevelMin = powerManager.measureBattery();

            REQUIRE(batteryLevelMin == 0);
        }

        SECTION("return the correct battery percentage")
        {
            const auto analogVoltage = static_cast<unsigned int>(round((Configurations::batteryVoltageMax - 0.2) * 1000));

            mockArduino.Reset();
            When(Method(mockArduino, analogReadMilliVolts)).AlwaysReturn(analogVoltage);
            Fake(Method(mockArduino, delay));
            const auto expectedBatteryLevel = lround((analogVoltage / 1'000.0 - Configurations::batteryVoltageMin) / (Configurations::batteryVoltageMax - Configurations::batteryVoltageMin) * 100);

            const auto batteryLevel = powerManager.measureBattery();

            REQUIRE(batteryLevel == expectedBatteryLevel);
        }
    }
}
// NOLINTEND(cppcoreguidelines-avoid-do-while)