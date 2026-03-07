#pragma once

#include "./power-manager.service.interface.h"

class ILedService;

class PowerManagerService final : public IPowerManagerService
{
    ILedService &ledService;

    [[nodiscard]] unsigned char setupBatteryMeasurement() const;
    static void printWakeupReason();
    static void powerSensorOn();

public:
    explicit PowerManagerService(ILedService &_ledService);

    [[nodiscard]] unsigned char setup() const override;
    void goToSleep() const override;
    [[nodiscard]] unsigned char measureBattery() const override;
};