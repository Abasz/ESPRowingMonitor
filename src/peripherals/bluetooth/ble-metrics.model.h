#pragma once

#include "../../utils/configuration.h"

namespace BleMetricsModel
{
    struct BleMetricsData
    {
        unsigned long long revTime;
        Configurations::precision distance;
        unsigned long long strokeTime;
        unsigned short strokeCount;
        Configurations::precision avgStrokePower;
    };
}