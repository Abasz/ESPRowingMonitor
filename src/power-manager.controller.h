#pragma once

#include "power-manager.service.h"

class PowerManagerController
{
    PowerManagerService &powerManagerService;

    byte batteryLevel = 0;
    byte previousBatteryLevel = 0;

    unsigned long lastBatteryMeasurementTime = 0;

public:
    PowerManagerController(PowerManagerService &_powerManagerService);

    void begin();
    void update(unsigned long lastRevTime, bool isDeviceConnected);
    byte getBatteryLevel() const;
    byte getPreviousBatteryLevel() const;
    void setPreviousBatteryLevel();
};