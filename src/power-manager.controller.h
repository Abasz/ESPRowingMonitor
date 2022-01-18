#pragma once

class PowerManagerController
{
    static unsigned long const DEEP_SLEEP_TIMEOUT = 4 * 60 * 1000 * 1000;

    static byte const VOLTAGE_DIVIDER_RATIO = 2;
    static double constexpr BATTERY_VOLTAGE_MIN = 3.3 / VOLTAGE_DIVIDER_RATIO;
    static double constexpr BATTERY_VOLTAGE_MAX = 4.2 / VOLTAGE_DIVIDER_RATIO;

    volatile byte batteryLevel = 0;
    hw_timer_t *batteryMeasurementTimer = timerBegin(1, 80, true);

    void setupBatteryMeasurement();

public:
    PowerManagerController();

    void begin();
    void checkSleep(unsigned long lastRevTime, bool isDeviceConnected) const;
    void measureBattery();
    byte getBatteryLevel() const;
};