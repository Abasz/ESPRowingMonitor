#pragma once

#include "profiles/generic.board-profile.h"
#include "profiles/generic.rower-profile.h"
#include "utils/enums.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define DEFAULT_CPS_LOGGING_LEVEL ArduinoLogLevel::LogLevelTrace
#define DEFAULT_BLE_SERVICE BleServiceFlag::CpsService
#define ENABLE_WEBSOCKET_MONITOR true
#define ENABLE_WEBGUI false
#define ENABLE_BLE_SERVICE true
#define PORT 80

// NOLINTEND(cppcoreguidelines-macro-usage)