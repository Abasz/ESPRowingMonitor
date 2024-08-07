#pragma once

#include "../utils/configuration.h"
#include "./stroke.model.h"

class FlywheelService
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

    static void setup();
    bool hasDataChanged() const;
    RowingDataModels::FlywheelData getData();
    void processRotation(unsigned long now);
};