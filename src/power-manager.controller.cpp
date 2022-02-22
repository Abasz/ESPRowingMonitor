#include <Arduino.h>

#include "ArduinoLog.h"

#include "power-manager.controller.h"

PowerManagerController::PowerManagerController(PowerManagerService &_powerManagerService) : powerManagerService(_powerManagerService)
{
}

void PowerManagerController::begin()
{
    Log.infoln("Setting up power manager controller");
    powerManagerService.setup();
    batteryLevel = powerManagerService.measureBattery();
}

void PowerManagerController::update(unsigned long lastRevTime, bool isDeviceConnected)
{
    auto now = micros();
    if (!isDeviceConnected && now - lastRevTime > DEEP_SLEEP_TIMEOUT * 1000)
    {
        powerManagerService.goToSleep();
    }

    if (now - lastBatteryMeasurementTime > BATTERY_MEASUREMENT_FREQUENCY * 1000)
    {
        batteryLevel = powerManagerService.measureBattery();
        lastBatteryMeasurementTime = now;
    }
}

byte PowerManagerController::getBatteryLevel() const
{
    return batteryLevel;
}

byte PowerManagerController::getPreviousBatteryLevel() const
{
    return previousBatteryLevel;
}

void PowerManagerController::setPreviousBatteryLevel()
{
    previousBatteryLevel = batteryLevel;
}