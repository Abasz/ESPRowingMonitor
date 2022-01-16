#pragma once

class PowerManagerController
{
    static unsigned long const DEEP_SLEEP_TIMEOUT = 4 * 60 * 1000 * 1000;

public:
    PowerManagerController();

    void begin() const;
    void checkSleep(unsigned long lastRevTime, bool isDeviceConnected) const;
};