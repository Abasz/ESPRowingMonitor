#pragma once

#include "../utils/enums.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)
#define DEVICE_NAME KayakFirst
#define MODEL_NUMBER Orange
#define SERIAL_NUMBER "03172022/1"

// Hardware settings
#define IMPULSES_PER_REVOLUTION 3
#define FLYWHEEL_INERTIA 0.02527962
#define SPROCKET_RADIUS 2.7
#define CONCEPT_2_MAGIC_NUMBER 2.75

// Sensor signal filter settings
#define ROTATION_DEBOUNCE_TIME_MIN 7
#define ROWING_STOPPED_THRESHOLD_PERIOD 7'000

// Drag factor filter settings
#define GOODNESS_OF_FIT_THRESHOLD 0.94
#define MAX_DRAG_FACTOR_RECOVERY_PERIOD 5'000
#define LOWER_DRAG_FACTOR_THRESHOLD 10
#define UPPER_DRAG_FACTOR_THRESHOLD 200
#define DRAG_COEFFICIENTS_ARRAY_LENGTH 1

// Stroke phase detection filter settings
#define MINIMUM_POWERED_TORQUE 0
#define MINIMUM_DRAG_TORQUE 0.214
#define STROKE_DETECTION_TYPE STROKE_DETECTION_TORQUE
#define MINIMUM_RECOVERY_SLOPE_MARGIN 0.00001 // Only relevant if STROKE_DETECTION_TYPE is either BOTH or TORQUE
#define MINIMUM_RECOVERY_SLOPE 0              // Only relevant if STROKE_DETECTION_TYPE is either BOTH or SLOPE
#define MINIMUM_RECOVERY_TIME 145
#define MINIMUM_DRIVE_TIME 170
#define IMPULSE_DATA_ARRAY_LENGTH 7
// #define FLOATING_POINT_PRECISION PRECISION_DOUBLE

// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)