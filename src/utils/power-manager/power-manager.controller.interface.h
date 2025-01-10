#pragma once

class IPowerManagerController
{
protected:
    ~IPowerManagerController() = default;

public:
    virtual void begin() = 0;
    virtual void update(unsigned long lastImpulseTime, bool isDeviceConnected) = 0;
    virtual unsigned char getBatteryLevel() const = 0;
    virtual unsigned char getPreviousBatteryLevel() const = 0;
    virtual void setPreviousBatteryLevel() = 0;
};