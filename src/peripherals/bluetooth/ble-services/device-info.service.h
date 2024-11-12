#pragma once

#include "NimBLEDevice.h"

class DeviceInfoBleService
{
public:
    static NimBLEService *setup(NimBLEServer *server);
};
