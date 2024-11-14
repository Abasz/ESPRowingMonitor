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
        Configurations::precision distance = 0;
        unsigned long long lastRevTime = 0;
        unsigned long long lastStrokeTime = 0;
        unsigned short strokeCount = 0;
        unsigned int driveDuration = 0;
        unsigned int recoveryDuration = 0;
        Configurations::precision avgStrokePower = 0;
        Configurations::precision dragCoefficient = 0;
        std::vector<float> driveHandleForces;
    };
}