#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>

#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "soc/gpio_num.h"

#include "Arduino.h"
#include "ArduinoLog.h"

#include "globals.h"

#include "./power-manager.service.h"

#include "../../peripherals/led/led.service.interface.h"
#include "../configuration.h"

PowerManagerService::PowerManagerService(ILedService &_ledService)
    : ledService(_ledService)
{
}

unsigned char PowerManagerService::setup() const
{
    printWakeupReason();
    if constexpr (Configurations::hasSensorOnSwitchPinNumber)
    {
        powerSensorOn();
    }

    if constexpr (Configurations::batteryPinNumber != GPIO_NUM_NC)
    {

        const auto batteryLevel = setupBatteryMeasurement();

        Log.traceln("Setting up battery measurement on pin %d", Configurations::batteryPinNumber);
        Log.verboseln("Initial battery level measurement: %d%%", batteryLevel);

        return batteryLevel;
    }

    return 0;
}

void PowerManagerService::powerSensorOn()
{
    pinMode(Configurations::sensorOnSwitchPinNumber, OUTPUT);
    digitalWrite(Configurations::sensorOnSwitchPinNumber, HIGH);
}

void PowerManagerService::goToSleep() const
{
    Log.verboseln("Configure deep sleep mode");

    if constexpr (Configurations::hasWakeupPinNumber)
    {
        pinMode(Configurations::wakeupPinNumber, INPUT_PULLUP);
    }

    if constexpr (Configurations::hasSensorOnSwitchPinNumber)
    {
        digitalWrite(Configurations::sensorOnSwitchPinNumber, LOW);
        gpio_hold_en(Configurations::sensorOnSwitchPinNumber);
        gpio_deep_sleep_hold_en();
    }

    const auto wakeupPin = Configurations::hasWakeupPinNumber ? Configurations::wakeupPinNumber : Configurations::sensorPinNumber;
    const auto pinState = digitalRead(wakeupPin);

    Log.verboseln("Wake up pin status: %s", pinState == HIGH ? "HIGH" : "LOW");

    esp_sleep_enable_ext0_wakeup(wakeupPin, pinState == HIGH ? LOW : HIGH);
    rtc_gpio_pullup_en(wakeupPin);

    Log.infoln("Going to sleep mode");
    ledService.clear();
    Serial.flush();
    esp_deep_sleep_start();
}

unsigned char PowerManagerService::measureBattery() const
{
    std::array<float, Configurations::batteryLevelArrayLength> batteryLevels{};
    batteryLevels.fill(0);

    for (auto &batteryLevel : batteryLevels)
    {
        auto rawNewBatteryLevel = (static_cast<float>(analogReadMilliVolts(Configurations::batteryPinNumber)) / 1'000.0F - Configurations::batteryVoltageMin) / (Configurations::batteryVoltageMax - Configurations::batteryVoltageMin) * 100;

        rawNewBatteryLevel = std::min(std::max(rawNewBatteryLevel, 0.0F), 100.0F);

        batteryLevel = rawNewBatteryLevel;
    }

    const unsigned char mid = Configurations::batteryLevelArrayLength / 2;
    std::ranges::nth_element(begin(batteryLevels), std::next(begin(batteryLevels), mid + 1), end(batteryLevels));

    if constexpr (isOdd(batteryLevels.size()))
    {
        return std::lround(batteryLevels[mid]);
    }

    return std::lround((batteryLevels[mid] + *std::ranges::max_element(cbegin(batteryLevels), std::next(cbegin(batteryLevels), mid))) / 2);
}

unsigned char PowerManagerService::setupBatteryMeasurement() const
{
    pinMode(Configurations::batteryPinNumber, INPUT);

    delay(500);
    unsigned char i = 0;
    unsigned short sum = 0;
    for (; i < Configurations::initialBatteryLevelMeasurementCount; ++i)
    {
        sum += measureBattery();
        delay(100);
    }

    return std::lround((float)sum / (float)i);
}

void PowerManagerService::printWakeupReason()
{
    const auto wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_UNDEFINED:
        Log.infoln("Wakeup caused by resetting or powering up the device");
        break;
    case ESP_SLEEP_WAKEUP_EXT0:
        Log.infoln("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Log.infoln("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Log.infoln("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Log.infoln("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Log.infoln("Wakeup caused by ULP program");
        break;
    default:
        Log.infoln("Wakeup was not caused by deep sleep: %d", wakeup_reason);
        break;
    }
}
