#pragma once

#include "power-manager.service.h"

class PowerManagerController
{
    PowerManagerService &powerManagerService;

    unsigned char batteryLevel = 0;
    unsigned char previousBatteryLevel = 0;

    unsigned long lastBatteryMeasurementTime = 0;

public:
    explicit PowerManagerController(PowerManagerService &_powerManagerService);

    void begin();
    void update(unsigned long lastRevTime, bool isDeviceConnected);
    unsigned char getBatteryLevel() const;
    unsigned char getPreviousBatteryLevel() const;
    void setPreviousBatteryLevel();
};