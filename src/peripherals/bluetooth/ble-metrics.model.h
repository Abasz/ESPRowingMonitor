#pragma once

#include <string>

#include "esp_err.h"

#include "NimBLEDevice.h"

#include "../../utils/configuration.h"
#include "../../utils/macros.h"

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

// NOLINTBEGIN(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-vararg)
#define ASSERT_SETUP_FAILED(className)                                         \
    Serial.printf("%.*s has not been setup, restarting\n",                     \
                  static_cast<int>((className).length()), (className).data()); \
                                                                               \
    ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);

#define ASSERT_SETUP_CALLED(characteristic)  \
    if ((characteristic) == nullptr)         \
    {                                        \
        ASSERT_SETUP_FAILED(__CLASS_NAME__); \
    }
// NOLINTEND(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-vararg)