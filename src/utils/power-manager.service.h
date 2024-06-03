#pragma once

#include "./power-manager.service.interface.h"

class PowerManagerService final : public IPowerManagerService
{
    unsigned char setupBatteryMeasurement() const;
    static void printWakeupReason();
    static void powerSensorOn();

public:
    PowerManagerService();

    unsigned char setup() const override;
    void goToSleep() const override;
    unsigned char measureBattery() const override;
};