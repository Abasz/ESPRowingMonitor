#pragma once

#include <string>

#include "Arduino.h"

#include "macros.h"

using std::string;

class Configurations
{
public:
    static ArduinoLogLevel const defaultLogLevel = DEFAULT_CPS_LOGGING_LEVEL;
    static BleServiceFlag const defaultBleServiceFlag = DEFAULT_BLE_SERVICE;
    static bool const isBleSErviceEnabled = ENABLE_BLE_SERVICE;
    static inline string const deviceName = TOSTRING(DEVICE_NAME);

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
    typedef PRECISION precision;
    static StrokeDetectionType constexpr strokeDetectionType = STROKE_DETECTION;
    static double constexpr minimumPoweredTorque = MINIMUM_POWERED_TORQUE;
    static double constexpr minimumDragTorque = MINIMUM_DRAG_TORQUE;
    static double constexpr minimumRecoverySlopeMargin = MINIMUM_RECOVERY_SLOPE_MARGIN / 1e6;
    static double constexpr minimumRecoverySlope = MINIMUM_RECOVERY_SLOPE;
    static unsigned int const strokeDebounceTime = STROKE_DEBOUNCE_TIME * 1000;
    static unsigned char const impulseDataArrayLength = IMPULSE_DATA_ARRAY_LENGTH;
    // static unsigned char const rotationSmoothingFactor = ROTATION_SMOOTHING_FACTOR;

    // Network settings
    static inline string const ssid = TOSTRING(LOCAL_SSID);
    static inline string const passphrase = TOSTRING(PASSPHRASE);
    static unsigned char const port = PORT;
    static bool const isWebsocketEnabled = ENABLE_WEBSOCKET_MONITOR;
    static bool const isWebGUIEnabled = ENABLE_WEBGUI;

    // Device power management settings
    static gpio_num_t const batteryPinNumber = BATTERY_PIN_NUMBER;
    static unsigned char const voltageDividerRatio = VOLTAGE_DIVIDER_RATIO;
    static double constexpr batteryVoltageMin = BATTERY_VOLTAGE_MIN / Configurations::voltageDividerRatio;
    static double constexpr batteryVoltageMax = BATTERY_VOLTAGE_MAX / Configurations::voltageDividerRatio;
    static unsigned char const batteryLevelArrayLength = BATTERY_LEVEL_ARRAY_LENGTH;
    static unsigned char const initialBatteryLevelMeasurementCount = INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT;
    static unsigned int const batteryMeasurementFrequency = BATTERY_MEASUREMENT_FREQUENCY * 60 * 1000;
    static unsigned long const deepSleepTimeout = DEEP_SLEEP_TIMEOUT * 60 * 1000;
};