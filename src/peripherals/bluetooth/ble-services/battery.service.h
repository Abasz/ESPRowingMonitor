#pragma once

#include "NimBLEDevice.h"

class BatteryBleService
{
public:
    NimBLECharacteristic *characteristic = nullptr;

    NimBLEService *setup(NimBLEServer *server);
};
