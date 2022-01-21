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

void PowerManagerController::update(unsigned long lastRevTime, bool isDeviceConnected) const
{
    powerManagerService.checkSleep(lastRevTime, isDeviceConnected);
}

byte PowerManagerController::getBatteryLevel() const
{
    return powerManagerService.getBatteryLevel();
}