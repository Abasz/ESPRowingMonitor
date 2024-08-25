#include "ArduinoLog.h"

#include "./configuration.h"
#include "./power-manager.controller.h"

PowerManagerController::PowerManagerController(IPowerManagerService &_powerManagerService) : powerManagerService(_powerManagerService)
{
}

void PowerManagerController::begin()
{
    Log.infoln("Setting up power manager controller");
    batteryLevel = powerManagerService.setup();
}

void PowerManagerController::update(const unsigned long lastRevTime, const bool isDeviceConnected)
{
    const auto now = micros();
    if (!isDeviceConnected && now - lastRevTime > Configurations::deepSleepTimeout * 1'000)
    {
        return powerManagerService.goToSleep();
    }

    if constexpr (Configurations::batteryPinNumber != GPIO_NUM_NC)
    {
        if (now - lastBatteryMeasurementTime > Configurations::batteryMeasurementFrequency * 1'000)
        {
            batteryLevel = powerManagerService.measureBattery();
            lastBatteryMeasurementTime = now;
        }
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