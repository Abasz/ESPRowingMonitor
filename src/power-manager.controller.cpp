#include <Arduino.h>

#include "globals.h"
#include "power-manager.controller.h"

PowerManagerController::PowerManagerController()
{
}

void PowerManagerController::begin()
{
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_33, ESP_EXT1_WAKEUP_ALL_LOW);
    gpio_hold_en(GPIO_NUM_33);
    gpio_deep_sleep_hold_en();

    setupBatteryMeasurement();
}

void PowerManagerController::checkSleep(unsigned long lastRevTime, bool isDeviceConnected) const
{
    if (!isDeviceConnected && micros() - lastRevTime > DEEP_SLEEP_TIMEOUT)
    {
        Serial.println("Going to sleep mode");
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
    pinMode(GPIO_NUM_4, INPUT);

    delay(500);
    auto sum = 0U;
    for (auto i = 0; i < 10; i++)
    {
        measureBattery();
        delay(100);
    }

    timerAttachInterrupt(batteryMeasurementTimer, batteryMeasurementInterrupt, true);
    timerAlarmWrite(batteryMeasurementTimer, 1000000 * 10, true);
    timerAlarmEnable(batteryMeasurementTimer);
}