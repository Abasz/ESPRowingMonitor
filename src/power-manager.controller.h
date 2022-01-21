#pragma once

#include "power-manager.service.h"

class PowerManagerController
{
    PowerManagerService &powerManagerService;

public:
    PowerManagerController(PowerManagerService &_powerManagerService);

    void begin();
    void update(unsigned long lastRevTime, bool isDeviceConnected) const;
    byte getBatteryLevel() const;
};