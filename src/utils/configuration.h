#pragma once

#include <string>

#include "Arduino.h"

#include "macros.h"

using std::string;

class Configurations
{
public:
    typedef PRECISION precision;

    static const bool logCalibration = LOG_CALIBRATION;
    static const ArduinoLogLevel defaultLogLevel = DEFAULT_CPS_LOGGING_LEVEL;
    static const BleServiceFlag defaultBleServiceFlag = DEFAULT_BLE_SERVICE;
    static const bool isBleServiceEnabled = ENABLE_BLE_SERVICE;
    static const BleSignalStrength bleSignalStrength = BLE_SIGNAL_STRENGTH;

    inline static const string deviceName = TOSTRING(DEVICE_NAME);
    inline static const string modelNumber = TOSTRING(MODEL_NUMBER);
    inline static const string serialNumber = SERIAL_NUMBER;
    inline static const string softwareVersion = std::to_string(compileYear) + std::to_string(compileMonth) + std::to_string(compileDay);

    // Hardware settings
    static const BaudRates baudRate = BAUD_RATE;
    static const gpio_num_t sensorPinNumber = SENSOR_PIN_NUMBER;
    static const gpio_num_t wakeupPinNumber = WAKEUP_SENSOR_PIN_NUMBER;
    static const bool hasWakeupPinNumber = Configurations::wakeupPinNumber != GPIO_NUM_NC;
    static const gpio_num_t sensorOnSwitchPinNumber = SENSOR_ON_SWITCH_PIN_NUMBER;
    static const bool hasSensorOnSwitchPinNumber = Configurations::sensorOnSwitchPinNumber != GPIO_NUM_NC;
    static const gpio_num_t ledPin = static_cast<gpio_num_t>(LED_PIN);
    static const bool isRgb = Configurations::ledPin == GPIO_NUM_NC ? false : IS_RGB;
    static const unsigned char impulsesPerRevolution = IMPULSES_PER_REVOLUTION;
    static constexpr double flywheelInertia = FLYWHEEL_INERTIA;
    static const unsigned short ledBlinkFrequency = LED_BLINK_FREQUENCY;
    static constexpr double sprocketRadius = SPROCKET_RADIUS / 100;
    static constexpr double concept2MagicNumber = CONCEPT_2_MAGIC_NUMBER;

    static constexpr Configurations::precision angularDisplacementPerImpulse = (2 * PI) / Configurations::impulsesPerRevolution;

    // Sensor signal filter settings
    static const unsigned short rotationDebounceTimeMin = ROTATION_DEBOUNCE_TIME_MIN * 1000;
    static const unsigned int rowingStoppedThresholdPeriod = ROWING_STOPPED_THRESHOLD_PERIOD * 1000;

    // Drag factor filter settings
    static constexpr double goodnessOfFitThreshold = GOODNESS_OF_FIT_THRESHOLD;
    static const unsigned int maxDragFactorRecoveryPeriod = MAX_DRAG_FACTOR_RECOVERY_PERIOD * 1000;
    static constexpr double lowerDragFactorThreshold = LOWER_DRAG_FACTOR_THRESHOLD / 1e6;
    static constexpr double upperDragFactorThreshold = UPPER_DRAG_FACTOR_THRESHOLD / 1e6;
    static const unsigned char dragCoefficientsArrayLength = DRAG_COEFFICIENTS_ARRAY_LENGTH;

    // Stroke phase detection filter settings
    static constexpr StrokeDetectionType strokeDetectionType = STROKE_DETECTION;
    static constexpr double minimumPoweredTorque = MINIMUM_POWERED_TORQUE;
    static constexpr double minimumDragTorque = MINIMUM_DRAG_TORQUE;
    static constexpr double minimumRecoverySlopeMargin = MINIMUM_RECOVERY_SLOPE_MARGIN / 1e6;
    static constexpr double minimumRecoverySlope = MINIMUM_RECOVERY_SLOPE;
    static const unsigned int minimumRecoveryTime = MINIMUM_RECOVERY_TIME * 1000;
    static const unsigned int minimumDriveTime = MINIMUM_DRIVE_TIME * 1000;
    static const unsigned char impulseDataArrayLength = IMPULSE_DATA_ARRAY_LENGTH;

    // Network settings
    inline static const string ssid = TOSTRING(LOCAL_SSID);
    inline static const string passphrase = TOSTRING(PASSPHRASE);
    static const unsigned char port = PORT;
    static const bool isWebsocketEnabled = ENABLE_WEBSOCKET_MONITOR;
    static const bool isWebGUIEnabled = ENABLE_WEBGUI;

    // Device power management settings
    static const gpio_num_t batteryPinNumber = BATTERY_PIN_NUMBER;
    static const unsigned char voltageDividerRatio = VOLTAGE_DIVIDER_RATIO;
    static constexpr double batteryVoltageMin = BATTERY_VOLTAGE_MIN / Configurations::voltageDividerRatio;
    static constexpr double batteryVoltageMax = BATTERY_VOLTAGE_MAX / Configurations::voltageDividerRatio;
    static const unsigned char batteryLevelArrayLength = BATTERY_LEVEL_ARRAY_LENGTH;
    static const unsigned char initialBatteryLevelMeasurementCount = INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT;
    static const unsigned int batteryMeasurementFrequency = BATTERY_MEASUREMENT_FREQUENCY * 60 * 1000;
    static const unsigned long deepSleepTimeout = DEEP_SLEEP_TIMEOUT * 60 * 1000;
};