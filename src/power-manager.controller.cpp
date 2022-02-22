#include <Arduino.h>

#include "ArduinoLog.h"

#include "globals.h"
#include "power-manager.controller.h"

PowerManagerController::PowerManagerController(PowerManagerService &_powerManagerService) : powerManagerService(_powerManagerService)
{
}

void PowerManagerController::begin()
{
    Log.infoln("Setting up power manager controller");
    powerManagerService.setup();
}

void PowerManagerController::update(unsigned long lastRevTime, bool isDeviceConnected)
{
    powerManagerService.checkSleep(lastRevTime, isDeviceConnected);
    batteryLevel = powerManagerService.getBatteryLevel();
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