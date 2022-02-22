#pragma once

#include "power-manager.service.h"

class PowerManagerController
{
    static unsigned int const BATTERY_MEASUREMENT_FREQUENCY = 10 * 60 * 1000;
    static unsigned long const DEEP_SLEEP_TIMEOUT = 4 * 60 * 1000;

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