#pragma once

#include <vector>

#include "../utils/configuration.h"

namespace RowingDataModels
{
    struct FlywheelData
    {
        unsigned long rawImpulseCount;
        unsigned long deltaTime;
        unsigned long long totalTime;
        Configurations::precision totalAngularDisplacement;
        unsigned long cleanImpulseTime;
        unsigned long rawImpulseTime;
    };

    struct RowingMetrics
    {
        Configurations::precision distance;
        unsigned long long lastRevTime;
        unsigned long long lastStrokeTime;
        unsigned short strokeCount;
        unsigned int driveDuration;
        unsigned int recoveryDuration;
        Configurations::precision avgStrokePower;
        Configurations::precision dragCoefficient;
        std::vector<Configurations::precision> driveHandleForces;
    };
}