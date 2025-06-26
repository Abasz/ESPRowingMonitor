#pragma once

#include "../utils/enums.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)
#define ENABLE_RUNTIME_SETTINGS true

#define DEVICE_NAME OldDanube
// clang-format off
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define MODEL_NUMBER 2025/6M
// clang-format on

#define ADD_SERIAL_TO_DEVICE_NAME true
#define ADD_BLE_SERVICE_TO_DEVICE_NAME false

// Hardware settings
#define IMPULSES_PER_REVOLUTION 6
#define FLYWHEEL_INERTIA 0.0894
#define SPROCKET_RADIUS 3.2
#define CONCEPT_2_MAGIC_NUMBER 2.57 // 0.1302 weight to magic number scaler

// Sensor signal filter settings
#define ROTATION_DEBOUNCE_TIME_MIN 4
#define ROWING_STOPPED_THRESHOLD_PERIOD 7'000

// Drag factor filter settings
#define GOODNESS_OF_FIT_THRESHOLD 0.67
#define MAX_DRAG_FACTOR_RECOVERY_PERIOD 4'000
#define LOWER_DRAG_FACTOR_THRESHOLD 10
#define UPPER_DRAG_FACTOR_THRESHOLD 200
#define DRAG_COEFFICIENTS_ARRAY_LENGTH 5

// Stroke phase detection filter settings
#define MINIMUM_POWERED_TORQUE 0
#define MINIMUM_DRAG_TORQUE 0.06
#define STROKE_DETECTION_TYPE STROKE_DETECTION_TORQUE
#define MINIMUM_RECOVERY_SLOPE_MARGIN 0.000001 // Only relevant if STROKE_DETECTION_TYPE is either BOTH or TORQUE
#define MINIMUM_RECOVERY_SLOPE 0               // Only relevant if STROKE_DETECTION_TYPE is either BOTH or SLOPE
#define MINIMUM_RECOVERY_TIME 145
#define MINIMUM_DRIVE_TIME 160
#define IMPULSE_DATA_ARRAY_LENGTH 10
#define DRIVE_HANDLE_FORCES_MAX_CAPACITY 59
// #define FLOATING_POINT_PRECISION PRECISION_DOUBLE

// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)