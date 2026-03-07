#pragma once

#include "./battery.service.interface.h"

class NimBLECharacteristic;
class NimBLEServer;
class NimBLEService;

class BatteryBleService final : public IBatteryBleService
{
    NimBLECharacteristic *characteristic = nullptr;

public:
    NimBLEService *setup(NimBLEServer *server) override;

    void broadcastBatteryLevel() const override;
    void setBatteryLevel(unsigned char batteryLevel) const override;
};
