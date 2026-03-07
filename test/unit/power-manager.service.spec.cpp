// NOLINTBEGIN(readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while, clang-analyzer-cplusplus.NewDeleteLeaks, clang-analyzer-cplusplus.NewDelete)
#include <cmath>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "soc/gpio_num.h"

#include "./include/Arduino.h"

#include "../../src/peripherals/led/led.service.interface.h"
#include "../../src/utils/configuration.h"
#include "../../src/utils/enums.h"
#include "../../src/utils/power-manager/power-manager.service.h"

using namespace fakeit;

TEST_CASE("PowerManagerService", "[utils]")
{
    mockArduino.Reset();

    Mock<ILedService> mockLedService;
    Fake(Method(mockLedService, setColor));
    When(Method(mockLedService, getColor)).AlwaysReturn(LedColor::Black);
    Fake(Method(mockLedService, refresh));
    Fake(Method(mockLedService, clear));

    PowerManagerService powerManager(mockLedService.get());
    const auto analogVoltage = static_cast<unsigned int>(std::round((Configurations::batteryVoltageMax - 0.1) * 1000));

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

            const unsigned char expectedBatteryLevel = std::lround((analogVoltage / 1'000.0 - Configurations::batteryVoltageMin) / (Configurations::batteryVoltageMax - Configurations::batteryVoltageMin) * 100);

            REQUIRE(batteryLevel == expectedBatteryLevel);
        }
    }

    SECTION("goToSleep method should")
    {
        const auto wakeupPinState = HIGH;

        mockArduino.Reset();
        mockSerial.Reset();
        Fake(Method(mockLedService, clear));
        When(Method(mockArduino, digitalRead)).Return(wakeupPinState);
        Fake(Method(mockArduino, pinMode));
        Fake(Method(mockArduino, digitalWrite));
        Fake(Method(mockArduino, gpio_hold_en));
        Fake(Method(mockArduino, gpio_deep_sleep_hold_en));
        Fake(Method(mockArduino, esp_sleep_enable_ext0_wakeup));
        Fake(Method(mockArduino, rtc_gpio_pullup_en));
        Fake(Method(mockArduino, esp_deep_sleep_start));
        Fake(Method(mockSerial, flush));

        powerManager.goToSleep();

        SECTION("set wakeup pin")
        {
            Verify(Method(mockArduino, pinMode).Using(Configurations::wakeupPinNumber, INPUT_PULLUP)).Once();
            Verify(Method(mockArduino, digitalWrite).Using(Configurations::sensorOnSwitchPinNumber, LOW)).Once();
            Verify(Method(mockArduino, digitalRead).Using(Configurations::wakeupPinNumber)).Once();
        }

        SECTION("configure deep sleep mode")
        {
            Verify(Method(mockArduino, rtc_gpio_pullup_en).Using(Configurations::wakeupPinNumber)).Once();
            Verify(Method(mockArduino, esp_sleep_enable_ext0_wakeup).Using(Configurations::wakeupPinNumber, !wakeupPinState)).Once();
        }

        SECTION("start deep sleep mode")
        {
            Verify(Method(mockLedService, clear)).Once();
            Verify(Method(mockSerial, flush)).Once();
            Verify(Method(mockArduino, esp_deep_sleep_start)).Once();
        }
    }

    SECTION("measureBattery method should")
    {
        const auto analogVoltageMax = std::lround((Configurations::batteryVoltageMax + 1) * 1000);
        const auto analogVoltageMin = std::lround((Configurations::batteryVoltageMin - 1) * 1000);

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
            mockArduino.Reset();
            When(Method(mockArduino, analogReadMilliVolts)).AlwaysReturn(analogVoltage);
            Fake(Method(mockArduino, delay));
            const auto expectedBatteryLevel = static_cast<unsigned char>(std::lround((analogVoltage / 1'000.0 - Configurations::batteryVoltageMin) / (Configurations::batteryVoltageMax - Configurations::batteryVoltageMin) * 100));

            REQUIRE(powerManager.measureBattery() == expectedBatteryLevel);
        }
    }
}
// NOLINTEND(readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while, clang-analyzer-cplusplus.NewDeleteLeaks, clang-analyzer-cplusplus.NewDelete)