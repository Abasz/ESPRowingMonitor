#pragma once

class PowerManagerService
{
    byte batteryLevel = 0;

    void setupBatteryMeasurement();
    void printWakeupReason() const;

public:
    PowerManagerService();

    void setup();
    void goToSleep() const;
    byte measureBattery();
};