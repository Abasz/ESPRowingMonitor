#pragma once

#include "../utils/configuration.h"
#include "./flywheel.service.interface.h"
#include "./stroke.model.h"

class FlywheelService final : public IFlywheelService
{
    volatile unsigned long lastDeltaTime = 0;
    volatile unsigned long cleanDeltaTime = 0;
    volatile unsigned long lastRawImpulseTime = 0;
    volatile unsigned long lastCleanImpulseTime = 0;
    volatile double totalAngularDisplacement = 0;

    volatile unsigned long impulseCount = 0UL;
    volatile unsigned long long totalTime = 0ULL;

    volatile bool isDataChanged = false;

public:
    FlywheelService();

    void setup() override;
    bool hasDataChanged() const override;
    RowingDataModels::FlywheelData getData() override;
    void processRotation(unsigned long now) override;
};