#pragma once

#include "NimBLEDevice.h"

#include "./battery.service.interface.h"

class BatteryBleService final : public IBatteryBleService
{
    NimBLECharacteristic *characteristic = nullptr;

public:
    NimBLEService *setup(NimBLEServer *server) override;

    void broadcastBatteryLevel() const override;
    void setBatteryLevel(unsigned char batteryLevel) const override;
};
