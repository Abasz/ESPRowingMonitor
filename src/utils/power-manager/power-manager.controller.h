#pragma once

#include "./power-manager.controller.interface.h"

class IPowerManagerService;

class PowerManagerController final : public IPowerManagerController
{
    IPowerManagerService &powerManagerService;

    unsigned char batteryLevel = 0;
    unsigned char previousBatteryLevel = 0;

    unsigned long lastBatteryMeasurementTime = 0;

public:
    explicit PowerManagerController(IPowerManagerService &_powerManagerService);

    void begin() override;
    void update(unsigned long lastImpulseTime, bool isDeviceConnected) override;
    [[nodiscard]] unsigned char getBatteryLevel() const override;
    [[nodiscard]] unsigned char getPreviousBatteryLevel() const override;
    void setPreviousBatteryLevel() override;
};