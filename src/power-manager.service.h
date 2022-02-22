#pragma once

class PowerManagerService
{
    static byte const VOLTAGE_DIVIDER_RATIO = 2;
    static double constexpr BATTERY_VOLTAGE_MIN = 3.3 / VOLTAGE_DIVIDER_RATIO;
    static double constexpr BATTERY_VOLTAGE_MAX = 4.00 / VOLTAGE_DIVIDER_RATIO;
    static byte const BATTERY_LEVEL_ARRAY_LENGTH = 5;
    static byte const INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT = 10;

    byte batteryLevel = 0;

    void setupBatteryMeasurement();
    void setupDeepSleep() const;
    void printWakeupReason() const;

public:
    PowerManagerService();

    void setup();
    void goToSleep() const;
    byte measureBattery();
};