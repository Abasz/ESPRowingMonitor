#pragma once

class IPowerManagerService
{
protected:
    ~IPowerManagerService() = default;

public:
    virtual unsigned char setup() const = 0;
    virtual void goToSleep() const = 0;
    virtual unsigned char measureBattery() const = 0;
};