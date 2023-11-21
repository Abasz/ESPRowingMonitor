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
using std::sort;

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
    setupBatteryMeasurement();
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

    auto const wakeupPin = Configurations::hasWakeupPinNumber ? Configurations::wakeupPinNumber : Configurations::sensorPinNumber;
    auto const pinState = digitalRead(wakeupPin);

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
    // execution time: 460 micro sec
    // auto start = micros();
    array<double, Configurations::batteryLevelArrayLength> batteryLevels{};

    for (unsigned char i = 0; i < Configurations::batteryLevelArrayLength; i++)
    {
        auto const measurement = analogRead(Configurations::batteryPinNumber);

        auto const espRefVolt = 3.3;
        auto const dacResolution = 4095;
        auto rawNewBatteryLevel = ((measurement * espRefVolt / dacResolution) - Configurations::batteryVoltageMin) / (Configurations::batteryVoltageMax - Configurations::batteryVoltageMin) * 100;

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

    batteryLevel = batteryLevel == 0 ? lround(batteryLevels[Configurations::batteryLevelArrayLength / 2]) : lround((batteryLevels[Configurations::batteryLevelArrayLength / 2] + batteryLevel) / 2);

    return batteryLevel;
    // auto stop = micros();
    // Serial.print("battery level: ");
    // Serial.println(stop - start);
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
    auto const wakeup_reason = esp_sleep_get_wakeup_cause();

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
