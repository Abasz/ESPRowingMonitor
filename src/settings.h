#pragma once

#include "enums.h"

#define DEFAULT_CPS_LOGGING_LEVEL ArduinoLogLevel::LogLevelTrace
#define DEFAULT_BLE_SERVICE BleServiceFlag::CpsService

// Hardware settings
#define SENSOR_PIN_NUMBER GPIO_NUM_26
#define IMPULSES_PER_REVOLUTION 3
#define FLYWHEEL_INERTIA 0.073
#define LED_BLINK_FREQUENCY 1000

// Sensor signal filter settings
#define ROTATION_DEBOUNCE_TIME_MIN 7
#define ROWING_STOPPED_THRESHOLD_PERIOD 7000

// Drag factor filter settings
#define DRAG_FACTOR_ROTATION_DELTA_UPPER_THRESHOLD 0
#define GOODNESS_OF_FIT_THRESHOLD 0.97
#define MAX_DRAG_FACTOR_RECOVERY_PERIOD 6000
#define LOWER_DRAG_FACTOR_THRESHOLD 75
#define UPPER_DRAG_FACTOR_THRESHOLD 250
#define DRAG_COEFFICIENTS_ARRAY_LENGTH 1

// Stroke phase detection filter settings
#define MIN_DECELERATION_DELTA_FOR_UNPOWERED 0
#define MAX_DECELERATION_DELTA_FOR_POWERED 0
#define STROKE_DEBOUNCE_TIME 200
#define FLYWHEEL_POWER_CHANGE_DETECTION_ERROR_THRESHOLD 0
#define DELTA_IMPULSE_TIME_ARRAY_LENGTH 5
#define ROTATION_SMOOTHING_FACTOR 1

// Device power management settings
#define VOLTAGE_DIVIDER_RATIO 2
#define BATTERY_VOLTAGE_MIN 3.3
#define BATTERY_VOLTAGE_MAX 4.00
#define BATTERY_LEVEL_ARRAY_LENGTH 5
#define INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT 10
#define BATTERY_MEASUREMENT_FREQUENCY 10
#define DEEP_SLEEP_TIMEOUT 4

class Settings
{
public:
    static ArduinoLogLevel const defaultLogLevel = DEFAULT_CPS_LOGGING_LEVEL;
    static BleServiceFlag const defaultBleServiceFlag = DEFAULT_BLE_SERVICE;

    // Hardware settings
    static gpio_num_t const sensorPinNumber = SENSOR_PIN_NUMBER;
    static unsigned char const impulsesPerRevolution = IMPULSES_PER_REVOLUTION;
    static double constexpr flywheelInertia = FLYWHEEL_INERTIA;
    static unsigned short const ledBlinkFrequency = LED_BLINK_FREQUENCY;

    // Sensor signal filter settings
    static unsigned char const rotationDebounceTimeMin = ROTATION_DEBOUNCE_TIME_MIN;
    static unsigned short const rowingStoppedThresholdPeriod = ROWING_STOPPED_THRESHOLD_PERIOD;

    // Drag factor filter settings
    static unsigned char const dragFactorRotationDeltaUpperThreshold = DRAG_FACTOR_ROTATION_DELTA_UPPER_THRESHOLD;
    static double constexpr goodnessOfFitThreshold = GOODNESS_OF_FIT_THRESHOLD;
    static unsigned short const maxDragFactorRecoveryPeriod = MAX_DRAG_FACTOR_RECOVERY_PERIOD;
    static double constexpr lowerDragFactorThreshold = LOWER_DRAG_FACTOR_THRESHOLD / 1e6;
    static double constexpr upperDragFactorThreshold = UPPER_DRAG_FACTOR_THRESHOLD / 1e6;
    static unsigned char const dragCoefficientsArrayLength = DRAG_COEFFICIENTS_ARRAY_LENGTH;

    // Stroke phase detection filter settings
    static unsigned short const minDecelerationDeltaForUnpowered = MIN_DECELERATION_DELTA_FOR_UNPOWERED;
    static unsigned short const maxDecelerationDeltaForPowered = MAX_DECELERATION_DELTA_FOR_POWERED;
    static unsigned char const strokeDebounceTime = STROKE_DEBOUNCE_TIME;
    static unsigned char const flywheelPowerChangeDetectionErrorThreshold = FLYWHEEL_POWER_CHANGE_DETECTION_ERROR_THRESHOLD;
    static unsigned char const deltaImpulseTimeArrayLength = DELTA_IMPULSE_TIME_ARRAY_LENGTH;
    static unsigned char const rotationSmoothingFactor = ROTATION_SMOOTHING_FACTOR;

    // Device power management settings
    static unsigned char const voltageDividerRatio = VOLTAGE_DIVIDER_RATIO;
    static double constexpr batteryVoltageMin = BATTERY_VOLTAGE_MIN / Settings::voltageDividerRatio;
    static double constexpr batteryVoltageMax = BATTERY_VOLTAGE_MAX / Settings::voltageDividerRatio;
    static unsigned char const batteryLevelArrayLength = BATTERY_LEVEL_ARRAY_LENGTH;
    static unsigned char const initialBatteryLevelMeasurementCount = INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT;
    static unsigned int const batteryMeasurementFrequency = BATTERY_MEASUREMENT_FREQUENCY * 60 * 1000;
    static unsigned long const deepSleepTimeout = DEEP_SLEEP_TIMEOUT * 60 * 1000;
};

// Sanity checks
#if DELTA_IMPULSE_TIME_ARRAY_LENGTH <= 1
    #error "DELTA_IMPULSE_TIME_ARRAY_LENGTH should not be less than 2"
#endif
#if DELTA_IMPULSE_TIME_ARRAY_LENGTH - FLYWHEEL_POWER_CHANGE_DETECTION_ERROR_THRESHOLD - 2 < 0
    #error "FLYWHEEL_POWER_CHANGE_DETECTION_ERROR_THRESHOLD should be less then or equal to the half of the DELTA_IMPULSE_TIME_ARRAY_LENGTH"
#endif