#pragma once

#include "../settings.h"
#include "stroke.model.h"

class FlywheelService
{
    static double constexpr angularDisplacementPerImpulse = (2 * PI) / Settings::impulsesPerRevolution;

    volatile unsigned long lastDeltaTime = 0;
    volatile unsigned long cleanDeltaTime = 0;
    volatile unsigned long lastRawImpulseTime = 0;
    volatile unsigned long lastCleanImpulseTime = 0;
    volatile double totalAngularDisplacement = 0.0;

    volatile unsigned long impulseCount = 0UL;
    unsigned long long totalTime = 0ULL;

    volatile bool isDataChanged = false;

public:
    FlywheelService();

    void setup() const;
    bool hasDataChanged() const;
    StrokeModel::FlywheelData getData();
    void processRotation(unsigned long now);
};