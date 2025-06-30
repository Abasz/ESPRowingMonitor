#pragma once

#include "../../../src/utils/enums.h"
#include "./test.board-profile.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)

#define ENABLE_RUNTIME_SETTINGS true
#define DEFAULT_CPS_LOGGING_LEVEL ArduinoLogLevel::LogLevelSilent
#define DEFAULT_BLE_SERVICE BleServiceFlag::CpsService

#define DEVICE_NAME TestDevice
#define MODEL_NUMBER TestModelNum
#define SERIAL_NUMBER "TestSerial"

#undef BAUD_RATE
#define BAUD_RATE BaudRates::Baud1500000
#define ENABLE_BLUETOOTH_DELTA_TIME_LOGGING true
#define SUPPORT_SD_CARD_LOGGING true

// Machine settings
#define IMPULSES_PER_REVOLUTION 3
#define FLYWHEEL_INERTIA 0.073
#define SPROCKET_RADIUS 1.50
#define CONCEPT_2_MAGIC_NUMBER 2.8

// Sensor signal filter settings
#define ROTATION_DEBOUNCE_TIME_MIN 7
#define ROWING_STOPPED_THRESHOLD_PERIOD 7'000

// Drag factor filter settings
#define GOODNESS_OF_FIT_THRESHOLD 0.97
#define MAX_DRAG_FACTOR_RECOVERY_PERIOD 6'000
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

// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)