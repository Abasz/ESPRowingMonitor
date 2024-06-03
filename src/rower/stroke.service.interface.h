#pragma once

#include "./stroke.model.h"

class IStrokeService
{
protected:
    ~IStrokeService() = default;

public:
    virtual RowingDataModels::RowingMetrics getData() = 0;
    virtual void processData(RowingDataModels::FlywheelData data) = 0;
};