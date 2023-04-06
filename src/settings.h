#pragma once

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define VAL(str) #str
#define TOSTRING(str) VAL(str)
#define EMPTY(...) (true __VA_OPT__(&&false))
// NOLINTEND(cppcoreguidelines-macro-usage)

#include <string>

#include "Arduino.h"

#include "utils/enums.h"

using std::string;

#define DEFAULT_CPS_LOGGING_LEVEL ArduinoLogLevel::LogLevelTrace
#define DEFAULT_BLE_SERVICE BleServiceFlag::CpsService

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// Hardware settings
#define SENSOR_PIN_NUMBER GPIO_NUM_26
#define IMPULSES_PER_REVOLUTION 3
#define FLYWHEEL_INERTIA 0.073
#define LED_BLINK_FREQUENCY 1000
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
#define MINIMUM_RECOVERY_SLOPE_MARGIN 0.0000022
#define STROKE_DEBOUNCE_TIME 300
#define IMPULSE_DATA_ARRAY_LENGTH 7

// Network settings
#define PORT 80

// Device power management settings
#define BATTERY_PIN_NUMBER GPIO_NUM_4
#define VOLTAGE_DIVIDER_RATIO 2
#define BATTERY_VOLTAGE_MIN 3.3
#define BATTERY_VOLTAGE_MAX 4.00
#define BATTERY_LEVEL_ARRAY_LENGTH 5
#define INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT 10
#define BATTERY_MEASUREMENT_FREQUENCY 10
#define DEEP_SLEEP_TIMEOUT 4
// NOLINTEND(cppcoreguidelines-macro-usage)
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
    static double constexpr sprocketRadius = SPROCKET_RADIUS;
    static double constexpr concept2MagicNumber = CONCEPT_2_MAGIC_NUMBER;

    // Sensor signal filter settings
    static unsigned short const rotationDebounceTimeMin = ROTATION_DEBOUNCE_TIME_MIN * 1000;
    static unsigned int const rowingStoppedThresholdPeriod = ROWING_STOPPED_THRESHOLD_PERIOD * 1000;

    // Drag factor filter settings
    static double constexpr goodnessOfFitThreshold = GOODNESS_OF_FIT_THRESHOLD;
    static unsigned int const maxDragFactorRecoveryPeriod = MAX_DRAG_FACTOR_RECOVERY_PERIOD * 1000;
    static double constexpr lowerDragFactorThreshold = LOWER_DRAG_FACTOR_THRESHOLD / 1e6;
    static double constexpr upperDragFactorThreshold = UPPER_DRAG_FACTOR_THRESHOLD / 1e6;
    static unsigned char const dragCoefficientsArrayLength = DRAG_COEFFICIENTS_ARRAY_LENGTH;

    // Stroke phase detection filter settings
    static double constexpr minimumPoweredTorque = MINIMUM_POWERED_TORQUE;
    static double constexpr minimumDragTorque = MINIMUM_DRAG_TORQUE;
    static double constexpr minimumRecoverySlopeMargin = MINIMUM_RECOVERY_SLOPE_MARGIN / 1e6;
    static unsigned int const strokeDebounceTime = STROKE_DEBOUNCE_TIME * 1000;
    static unsigned char const impulseDataArrayLength = IMPULSE_DATA_ARRAY_LENGTH;
    // static unsigned char const rotationSmoothingFactor = ROTATION_SMOOTHING_FACTOR;

    // Network settings
    static inline string const ssid = TOSTRING(LOCAL_SSID);
    static inline string const passphrase = TOSTRING(PASSPHRASE);
    static unsigned char const port = PORT;

    // Device power management settings
    static gpio_num_t const batteryPinNumber = BATTERY_PIN_NUMBER;
    static unsigned char const voltageDividerRatio = VOLTAGE_DIVIDER_RATIO;
    static double constexpr batteryVoltageMin = BATTERY_VOLTAGE_MIN / Settings::voltageDividerRatio;
    static double constexpr batteryVoltageMax = BATTERY_VOLTAGE_MAX / Settings::voltageDividerRatio;
    static unsigned char const batteryLevelArrayLength = BATTERY_LEVEL_ARRAY_LENGTH;
    static unsigned char const initialBatteryLevelMeasurementCount = INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT;
    static unsigned int const batteryMeasurementFrequency = BATTERY_MEASUREMENT_FREQUENCY * 60 * 1000;
    static unsigned long const deepSleepTimeout = DEEP_SLEEP_TIMEOUT * 60 * 1000;
};

// Sanity checks
#if IMPULSE_DATA_ARRAY_LENGTH < 3
    #error "IMPULSE_DATA_ARRAY_LENGTH should not be less than 3"
#endif
#if IMPULSE_DATA_ARRAY_LENGTH >= 12 && IMPULSE_DATA_ARRAY_LENGTH < 15
    #warning "Consider changing doubles to float "
#endif
#if IMPULSE_DATA_ARRAY_LENGTH >= 15 && IMPULSE_DATA_ARRAY_LENGTH < 18
    #error "Using too many data points will increase loop execution time. Using 15 and doubles would require approx. 7ms to complete calculation. Hence impulses may be missed. Change to floats to save on execution time (but potentially loose some precision)"
#endif
#if IMPULSE_DATA_ARRAY_LENGTH >= 18
    #error "Using too many data points will increase loop execution time. It should not be more than 17"
#endif
#if (!defined(LOCAL_SSID) || !defined(PASSPHRASE))
    #define DISABLE_WIFI_MONITOR
    #error "Not provided SSID and/or Passphrase, disabling wifi monitor"
#endif