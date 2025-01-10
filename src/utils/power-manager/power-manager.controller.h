#pragma once

#include "./power-manager.controller.interface.h"
#include "./power-manager.service.interface.h"

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
    unsigned char getBatteryLevel() const override;
    unsigned char getPreviousBatteryLevel() const override;
    void setPreviousBatteryLevel() override;
};