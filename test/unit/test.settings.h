#pragma once

#include "../src/profiles/generic.board-profile.h"

#include "../src/utils/enums.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define DEFAULT_CPS_LOGGING_LEVEL ArduinoLogLevel::LogLevelTrace
#define DEFAULT_BLE_SERVICE BleServiceFlag::CpsService
#define ENABLE_WEBSOCKET_MONITOR true
#define ENABLE_WEBGUI false
#define ENABLE_BLE_SERVICE true
#define PORT 80

#undef BAUD_RATE
#define BAUD_RATE BaudRates::Baud1500000
// Machine settings
#define IMPULSES_PER_REVOLUTION 3
#define FLYWHEEL_INERTIA 0.073
#define SPROCKET_RADIUS 1.50
#define CONCEPT_2_MAGIC_NUMBER 2.8

// Sensor signal filter settings
#define ROTATION_DEBOUNCE_TIME_MIN 7
#define ROWING_STOPPED_THRESHOLD_PERIOD 7000

// Drag factor filter settings
#define GOODNESS_OF_FIT_THRESHOLD 0.97
#define MAX_DRAG_FACTOR_RECOVERY_PERIOD 6000
#define LOWER_DRAG_FACTOR_THRESHOLD 75
#define UPPER_DRAG_FACTOR_THRESHOLD 250
#define DRAG_COEFFICIENTS_ARRAY_LENGTH 1

// Stroke phase detection filter settings
#define MINIMUM_POWERED_TORQUE 0
#define MINIMUM_DRAG_TORQUE 0
#define STROKE_DETECTION_TYPE STROKE_DETECTION_TORQUE
#define MINIMUM_RECOVERY_SLOPE_MARGIN 0.00001 // Only relevant if STROKE_DETECTION_TYPE is either BOTH or TORQUE
#define MINIMUM_RECOVERY_SLOPE 0.01           // Only relevant if STROKE_DETECTION_TYPE is either BOTH or SLOPE
#define MINIMUM_RECOVERY_TIME 300
#define MINIMUM_DRIVE_TIME 300
#define IMPULSE_DATA_ARRAY_LENGTH 7
// #define FLOATING_POINT_PRECISION PRECISION_DOUBLE

// NOLINTEND(cppcoreguidelines-macro-usage)