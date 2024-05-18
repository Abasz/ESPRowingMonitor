#pragma once

class PowerManagerService
{
    static unsigned char setupBatteryMeasurement();
    static void printWakeupReason();
    static void powerSensorOn();

public:
    PowerManagerService();

    static unsigned char setup();
    static void goToSleep();
    static unsigned char measureBattery();
};