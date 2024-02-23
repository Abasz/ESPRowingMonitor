#include <algorithm>
#include <array>
#include <numeric>

#include "Arduino.h"

#include "ArduinoLog.h"
#include "FastLED.h"

#include "configuration.h"
#include "power-manager.service.h"

using std::accumulate;
using std::array;

PowerManagerService::PowerManagerService()
{
}

void PowerManagerService::setup()
{
    printWakeupReason();
    if constexpr (Configurations::hasSensorOnSwitchPinNumber)
    {
        powerSensorOn();
    }
    if constexpr (Configurations::batteryPinNumber != GPIO_NUM_NC)
    {
        setupBatteryMeasurement();
    }
}

void PowerManagerService::powerSensorOn()
{
    pinMode(Configurations::sensorOnSwitchPinNumber, OUTPUT);
    digitalWrite(Configurations::sensorOnSwitchPinNumber, HIGH);
}

void PowerManagerService::goToSleep()
{
    Log.verboseln("Configure deep sleep mode");

    if constexpr (Configurations::hasWakeupPinNumber)
    {
        pinMode(Configurations::wakeupPinNumber, INPUT_PULLUP);
    }

    if constexpr (Configurations::hasSensorOnSwitchPinNumber)
    {
        digitalWrite(Configurations::sensorOnSwitchPinNumber, LOW);
    }

    const auto wakeupPin = Configurations::hasWakeupPinNumber ? Configurations::wakeupPinNumber : Configurations::sensorPinNumber;
    const auto pinState = digitalRead(wakeupPin);

    Log.verboseln("Wake up pin status: %s", pinState == HIGH ? "HIGH" : "LOW");

    esp_sleep_enable_ext0_wakeup(wakeupPin, pinState == HIGH ? LOW : HIGH);

    Log.infoln("Going to sleep mode");
    if constexpr (Configurations::isRgb)
    {
        FastLED.clear(true);
    }
    Serial.flush();
    esp_deep_sleep_start();
}

unsigned char PowerManagerService::measureBattery()
{
    array<double, Configurations::batteryLevelArrayLength> batteryLevels{};

    for (unsigned char i = 0; i < Configurations::batteryLevelArrayLength; i++)
    {
        auto rawNewBatteryLevel = (analogReadMilliVolts(Configurations::batteryPinNumber) / 1'000.0 - Configurations::batteryVoltageMin) / (Configurations::batteryVoltageMax - Configurations::batteryVoltageMin) * 100;

        if (rawNewBatteryLevel > 100)
        {
            rawNewBatteryLevel = 100;
        }

        if (rawNewBatteryLevel < 0)
        {
            rawNewBatteryLevel = 0;
        }

        batteryLevels[i] = accumulate(cbegin(batteryLevels), cbegin(batteryLevels) + i, rawNewBatteryLevel) / (i + 1);
    }
    const unsigned char mid = Configurations::batteryLevelArrayLength / 2;
    std::partial_sort(begin(batteryLevels), begin(batteryLevels) + mid + 1, end(batteryLevels));
    const unsigned char median = batteryLevels.size() % 2 != 0
                                     ? lround(batteryLevels[mid])
                                     : lround((batteryLevels[mid - 1] + batteryLevels[mid]) / 2);

    batteryLevel = batteryLevel == 0 ? median : lround((median + batteryLevel) / 2);

    return batteryLevel;
}

void PowerManagerService::setupBatteryMeasurement()
{
    pinMode(Configurations::batteryPinNumber, INPUT);

    delay(500);
    for (unsigned char i = 0; i < Configurations::initialBatteryLevelMeasurementCount; i++)
    {
        measureBattery();
        delay(100);
    }
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
