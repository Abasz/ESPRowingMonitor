#include <array>
#include <algorithm>
#include <numeric>

#include <Arduino.h>

#include "ArduinoLog.h"

#include "power-manager.service.h"

using std::accumulate;
using std::array;
using std::sort;

PowerManagerService::PowerManagerService()
{
}

void PowerManagerService::setup()
{
    setupDeepSleep();
    setupBatteryMeasurement();
}

void PowerManagerService::goToSleep() const
{
    Log.infoln("Going to sleep mode");
    esp_deep_sleep_start();
}

byte PowerManagerService::measureBattery()
{
    // execution time: 460 micro sec
    // auto start = micros();
    array<double, BATTERY_LEVEL_ARRAY_LENGTH> batteryLevels{};

    for (byte i = 0; i < BATTERY_LEVEL_ARRAY_LENGTH; i++)
    {
        auto measurement = analogRead(GPIO_NUM_34);

        auto rawNewBatteryLevel = ((measurement * 3.3 / 4095) - BATTERY_VOLTAGE_MIN) / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN) * 100;

        if (rawNewBatteryLevel > 100)
        {
            rawNewBatteryLevel = 100;
        }

        if (rawNewBatteryLevel < 0)
        {
            rawNewBatteryLevel = 0;
        }

        batteryLevels[i] = accumulate(batteryLevels.cbegin(), batteryLevels.cbegin() + i, rawNewBatteryLevel) / (i + 1);
    }

    sort(batteryLevels.begin(), batteryLevels.end());

    batteryLevel = batteryLevel == 0 ? lround(batteryLevels[BATTERY_LEVEL_ARRAY_LENGTH / 2]) : lround((batteryLevels[BATTERY_LEVEL_ARRAY_LENGTH / 2] + batteryLevel) / 2);

    return batteryLevel;
    // auto stop = micros();
    // Serial.print("battery level: ");
    // Serial.println(stop - start);
}

void PowerManagerService::setupBatteryMeasurement()
{
    pinMode(GPIO_NUM_4, INPUT);

    delay(500);
    for (byte i = 0; i < INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT; i++)
    {
        measureBattery();
        delay(100);
    }
}

void PowerManagerService::setupDeepSleep() const
{
    printWakeupReason();
    Log.traceln("Configure deep sleep mode");
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, LOW);
    gpio_hold_en(GPIO_NUM_26);
}

void PowerManagerService::printWakeupReason() const
{
    auto wakeup_reason = esp_sleep_get_wakeup_cause();

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
