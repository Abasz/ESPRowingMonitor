#pragma once

#include "NimBLEDevice.h"

class IBatteryBleService
{
protected:
    ~IBatteryBleService() = default;

public:
    virtual NimBLEService *setup(NimBLEServer *server) = 0;
    virtual bool isSubscribed() const = 0;
    virtual void broadcastBatteryLevel() const = 0;
    virtual void setBatteryLevel(unsigned char batteryLevel) const = 0;
};
