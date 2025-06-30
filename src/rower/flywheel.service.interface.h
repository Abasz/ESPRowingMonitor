#pragma once

#include "../utils/settings.model.h"
#include "./stroke.model.h"

class IFlywheelService
{
protected:
    ~IFlywheelService() = default;

public:
    virtual void setup(RowerProfile::MachineSettings newMachineSettings, RowerProfile::SensorSignalSettings newSensorSignalSettings) = 0;
    virtual bool hasDataChanged() const = 0;
    virtual RowingDataModels::FlywheelData getData() = 0;
    virtual void processRotation(unsigned long now) = 0;
};