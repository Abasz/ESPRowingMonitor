#include <Arduino.h>

#include "ArduinoLog.h"

#include "globals.h"
#include "power-manager.service.h"

PowerManagerService::PowerManagerService()
{
}

void PowerManagerService::setup()
{
    setupDeepSleep();
    setupBatteryMeasurement();
}

void PowerManagerService::checkSleep(unsigned long lastRevTime, bool isDeviceConnected) const
{
    if (!isDeviceConnected && micros() - lastRevTime > DEEP_SLEEP_TIMEOUT)
    {
        Log.infoln("Going to sleep mode");
        esp_deep_sleep_start();
    }
}

byte PowerManagerService::getBatteryLevel() const
{
    return batteryLevel;
}

void PowerManagerService::measureBattery()
{
    // auto start = micros();
    // execution time: 90 micro sec
    auto measurement = analogRead(GPIO_NUM_4);
    auto newBatteryLevel = ((measurement * 3.3 / 4095) - BATTERY_VOLTAGE_MIN) / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN) * 100;

    if (newBatteryLevel > 100.0)
    {
        newBatteryLevel = 100;
    }

    if (newBatteryLevel < 0)
    {
        newBatteryLevel = 0;
    }

    batteryLevel = batteryLevel == 0 ? lround(newBatteryLevel) : lround((newBatteryLevel + batteryLevel) / 2);
    //     auto stop = micros();
    //     Serial.print("battery level: ");
    //     Serial.println(stop - start);
}

void PowerManagerService::setupBatteryMeasurement()
{
    pinMode(GPIO_NUM_4, INPUT);

    delay(500);
    for (byte i = 0; i < 10; i++)
    {
        measureBattery();
        delay(100);
    }

    Log.traceln("Attache battery measurement timer interrupt");

    timerAttachInterrupt(batteryMeasurementTimer, batteryMeasurementInterrupt, true);
    timerAlarmWrite(batteryMeasurementTimer, BATTERY_LEVEL_MEASUREMENT_FREQUENCY, true);
    timerAlarmEnable(batteryMeasurementTimer);
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
