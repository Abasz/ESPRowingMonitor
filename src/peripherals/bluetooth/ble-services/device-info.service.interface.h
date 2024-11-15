#pragma once

#include "NimBLEDevice.h"

class IDeviceInfoBleService
{
protected:
    ~IDeviceInfoBleService() = default;

public:
    virtual NimBLEService *setup(NimBLEServer *server) const = 0;
};
