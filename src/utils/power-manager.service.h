#pragma once

class PowerManagerService
{
    unsigned char batteryLevel = 0;

    void setupBatteryMeasurement();
    void printWakeupReason() const;

public:
    PowerManagerService();

    void setup();
    void goToSleep() const;
    unsigned char measureBattery();
};