#pragma once

#include "../../utils/configuration.h"

namespace BleMetricsModel
{
    struct BleMetricsData
    {
        unsigned long long revTime;
        unsigned long long previousRevTime;
        Configurations::precision distance;
        Configurations::precision previousDistance;
        unsigned long long strokeTime;
        unsigned long long previousStrokeTime;
        unsigned short strokeCount;
        unsigned short previousStrokeCount;
        Configurations::precision avgStrokePower;
        Configurations::precision dragCoefficient;
    };
}