#pragma once

class PowerManagerService
{
    unsigned char batteryLevel = 0;

    void setupBatteryMeasurement();
    static void printWakeupReason();
    static void powerSensorOn();

public:
    PowerManagerService();

    void setup();
    static void goToSleep();
    unsigned char measureBattery();
};