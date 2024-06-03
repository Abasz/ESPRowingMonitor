#pragma once

#include "../rower/stroke.model.h"

class IPeripheralsController
{
protected:
    ~IPeripheralsController() = default;

public:
    virtual void begin() = 0;
    virtual void update(unsigned char batteryLevel) = 0;
    virtual void notifyBattery(unsigned char batteryLevel) = 0;
    virtual void updateData(const RowingDataModels::RowingMetrics &data) = 0;
    virtual void updateDeltaTime(unsigned long deltaTime) = 0;
    virtual bool isAnyDeviceConnected() = 0;
};
