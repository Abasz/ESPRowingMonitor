#pragma once

#include "../utils/settings.model.h"
#include "./stroke.model.h"

class IStrokeService
{
protected:
    ~IStrokeService() = default;

public:
#if ENABLE_RUNTIME_SETTINGS
    virtual void setup(RowerProfile::MachineSettings newMachineSettings) = 0;
#endif
    virtual RowingDataModels::RowingMetrics getData() = 0;
    virtual void processData(RowingDataModels::FlywheelData data) = 0;
};