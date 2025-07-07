#pragma once

#include BOARD_PROFILE
#include ROWER_PROFILE

#include "ArduinoLog.h"

#include "./utils/enums.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define DEFAULT_CPS_LOGGING_LEVEL ArduinoLogLevel::LogLevelSilent
#define DEFAULT_BLE_SERVICE BleServiceFlag::CpsService
#define ENABLE_BLUETOOTH_DELTA_TIME_LOGGING true

// NOLINTEND(cppcoreguidelines-macro-usage)