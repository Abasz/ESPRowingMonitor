#include <Arduino.h>

#include "ArduinoLog.h"

#include "globals.h"
#include "power-manager.controller.h"

PowerManagerController::PowerManagerController()
{
}

void PowerManagerController::begin()
{
    Log.infoln("Setting up power manager controller");
    setupDeepSleep();
    setupBatteryMeasurement();
}

void PowerManagerController::checkSleep(unsigned long lastRevTime, bool isDeviceConnected) const
{
    if (!isDeviceConnected && micros() - lastRevTime > DEEP_SLEEP_TIMEOUT)
    {
        Log.infoln("Going to sleep mode");
        esp_deep_sleep_start();
    }
}

byte PowerManagerController::getBatteryLevel() const
{
    return batteryLevel;
}

void PowerManagerController::measureBattery()
{
    // auto start = micros();
    // execution time: 90 micro sec
    auto measurement = analogRead(GPIO_NUM_4);
    auto newBatteryLevel = (((measurement * 3.3) / 4095) - BATTERY_VOLTAGE_MIN) / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN) * 100;

    batteryLevel = batteryLevel == 0 ? lround(newBatteryLevel) : lround((newBatteryLevel + batteryLevel) / 2);
    //     auto stop = micros();
    //     Serial.print("battery level: ");
    //     Serial.println(stop - start);
}

void PowerManagerController::setupBatteryMeasurement()
{
    Log.traceln("Attache battery measurement timer interrupt");

    pinMode(GPIO_NUM_4, INPUT);

    delay(500);
    for (byte i = 0; i < 10; i++)
    {
        measureBattery();
        delay(100);
    }

    timerAttachInterrupt(batteryMeasurementTimer, batteryMeasurementInterrupt, true);
    timerAlarmWrite(batteryMeasurementTimer, BATTERY_LEVEL_MEASUREMENT_FREQUENCY, true);
    timerAlarmEnable(batteryMeasurementTimer);
}

void PowerManagerController::setupDeepSleep() const
{
    Log.traceln("Configure deep sleep mode");
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_26, ESP_EXT1_WAKEUP_ALL_LOW);
    gpio_hold_en(GPIO_NUM_26);
    gpio_deep_sleep_hold_en();
}