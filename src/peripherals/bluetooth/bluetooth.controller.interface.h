#pragma once

#include <vector>

class IBluetoothController
{
protected:
    ~IBluetoothController() = default;

public:
    virtual void setup() = 0;
    virtual void startBLEServer() = 0;
    virtual void stopServer() = 0;

    virtual void notifyBattery(unsigned char batteryLevel) const = 0;
    virtual void notifyBaseMetrics(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) = 0;
    virtual void notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor) = 0;
    virtual void notifyHandleForces(const std::vector<float> &handleForces) = 0;
    virtual void notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes) = 0;

    virtual unsigned short calculateDeltaTimesMtu() const = 0;
    virtual bool isAnyDeviceConnected() = 0;
};
