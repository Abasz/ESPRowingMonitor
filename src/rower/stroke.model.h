#pragma once

#include <vector>
// TODO: rename this to RowingModel or RowingEngingModel or somthing
namespace StrokeModel
{
    struct FlywheelData
    {
        unsigned long rawImpulseCount;
        unsigned long deltaTime;
        unsigned long long totalTime;
        double totalAngularDisplacement;
        unsigned long cleanImpulseTime;
        unsigned long rawImpulseTime;
    };

    struct RowingMetrics
    {
        double distance;
        unsigned long long lastRevTime;
        unsigned long long lastStrokeTime;
        unsigned short strokeCount;
        unsigned int driveDuration;
        unsigned int recoveryDuration;
        double avgStrokePower;
        double dragCoefficient;
        std::vector<double> driveHandleForces;
    };
}