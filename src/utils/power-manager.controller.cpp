#include "ArduinoLog.h"

#include "../settings.h"
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
    if (!isDeviceConnected && now - lastRevTime > Settings::deepSleepTimeout * 1000)
    {
        powerManagerService.goToSleep();
    }

    if (now - lastBatteryMeasurementTime > Settings::batteryMeasurementFrequency * 1000)
    {
        batteryLevel = powerManagerService.measureBattery();
        lastBatteryMeasurementTime = now;
    }
}

unsigned char PowerManagerController::getBatteryLevel() const
{
    return batteryLevel;
}

unsigned char PowerManagerController::getPreviousBatteryLevel() const
{
    return previousBatteryLevel;
}

void PowerManagerController::setPreviousBatteryLevel()
{
    previousBatteryLevel = batteryLevel;
}