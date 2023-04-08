#pragma once

#include <vector>

#include "../settings.h"

namespace RowingDataModels
{
    struct FlywheelData
    {
        unsigned long rawImpulseCount;
        unsigned long deltaTime;
        unsigned long long totalTime;
        Settings::precision totalAngularDisplacement;
        unsigned long cleanImpulseTime;
        unsigned long rawImpulseTime;
    };

    struct RowingMetrics
    {
        Settings::precision distance;
        unsigned long long lastRevTime;
        unsigned long long lastStrokeTime;
        unsigned short strokeCount;
        unsigned int driveDuration;
        unsigned int recoveryDuration;
        Settings::precision avgStrokePower;
        Settings::precision dragCoefficient;
        std::vector<Settings::precision> driveHandleForces;
    };
}