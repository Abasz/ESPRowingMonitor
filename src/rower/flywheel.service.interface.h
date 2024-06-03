#pragma once

#include "./stroke.model.h"

class IFlywheelService
{
protected:
    ~IFlywheelService() = default;

public:
    virtual void setup() = 0;
    virtual bool hasDataChanged() const = 0;
    virtual RowingDataModels::FlywheelData getData() = 0;
    virtual void processRotation(unsigned long now) = 0;
};