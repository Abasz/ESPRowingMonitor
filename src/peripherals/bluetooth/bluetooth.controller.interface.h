#pragma once

#include <vector>

#include "../../rower/stroke.model.h"

class IBluetoothController
{
protected:
    ~IBluetoothController() = default;

public:
    virtual void update() = 0;
    virtual void setup() = 0;
    virtual void startBLEServer() = 0;
    virtual void stopServer() = 0;

    virtual void notifyBattery(unsigned char batteryLevel) const = 0;
    virtual void notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes) = 0;
    virtual void notifyNewMetrics(const RowingDataModels::RowingMetrics &data) = 0;

    virtual unsigned short calculateDeltaTimesMtu() const = 0;
    virtual bool isAnyDeviceConnected() = 0;
};
