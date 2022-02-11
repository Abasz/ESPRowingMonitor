#pragma once

class PowerManagerService
{
    static unsigned long const DEEP_SLEEP_TIMEOUT = 4 * 60 * 1000 * 1000;
    static unsigned long const BATTERY_LEVEL_MEASUREMENT_FREQUENCY = 10 * 60 * 1000 * 1000;

    static byte const VOLTAGE_DIVIDER_RATIO = 2;
    static double constexpr BATTERY_VOLTAGE_MIN = 3.3 / VOLTAGE_DIVIDER_RATIO;
    static double constexpr BATTERY_VOLTAGE_MAX = 4.00 / VOLTAGE_DIVIDER_RATIO;
    static byte const BATTERY_LEVEL_ARRAY_LENGTH = 5;

    volatile byte batteryLevel = 0;
    hw_timer_t *const batteryMeasurementTimer = timerBegin(1, 80, true);

    void setupBatteryMeasurement();
    void setupDeepSleep() const;
    void printWakeupReason() const;

public:
    PowerManagerService();

    void setup();
    void checkSleep(unsigned long lastRevTime, bool isDeviceConnected) const;
    byte getBatteryLevel() const;
    void measureBattery();
};