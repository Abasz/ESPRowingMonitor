#pragma once

#include <climits>
#include <string>

#include "Arduino.h"
#include "FastLED.h"

#include "./macros.h"

using std::string;

class Configurations
{
public:
    typedef PRECISION precision;

    static constexpr bool logCalibration = LOG_CALIBRATION;
    static constexpr ArduinoLogLevel defaultLogLevel = DEFAULT_CPS_LOGGING_LEVEL;
    static constexpr bool supportSdCardLogging = SUPPORT_SD_CARD_LOGGING;

    // Bluetooth Settings
    static constexpr BleServiceFlag defaultBleServiceFlag = DEFAULT_BLE_SERVICE;
    static constexpr bool hasExtendedBleMetrics = HAS_BLE_EXTENDED_METRICS;
    static constexpr bool enableBluetoothDeltaTimeLogging = ENABLE_BLUETOOTH_DELTA_TIME_LOGGING;
    static constexpr BleSignalStrength bleSignalStrength = BLE_SIGNAL_STRENGTH;

    inline static const string deviceName = TOSTRING(DEVICE_NAME);
    inline static const string modelNumber = TOSTRING(MODEL_NUMBER);
    inline static const string serialNumber = SERIAL_NUMBER;
    inline static const string firmwareVersion = string(getCompileDate().data(), getCompileDate().size());

    // Hardware settings
    static constexpr BaudRates baudRate = BAUD_RATE;

    static constexpr gpio_num_t sdCardChipSelectPin = SD_CARD_CHIP_SELECT_PIN;
    static constexpr gpio_num_t sensorPinNumber = SENSOR_PIN_NUMBER;
    static constexpr gpio_num_t wakeupPinNumber = WAKEUP_SENSOR_PIN_NUMBER;
    static constexpr bool hasWakeupPinNumber = wakeupPinNumber != GPIO_NUM_NC;
    static constexpr gpio_num_t sensorOnSwitchPinNumber = SENSOR_ON_SWITCH_PIN_NUMBER;
    static constexpr bool hasSensorOnSwitchPinNumber = sensorOnSwitchPinNumber != GPIO_NUM_NC;

    static constexpr gpio_num_t ledPin = static_cast<gpio_num_t>(LED_PIN);
    // NOLINTBEGIN(bugprone-branch-clone,readability-simplify-boolean-expr)
    static constexpr bool isRgb = ledPin == GPIO_NUM_NC ? false : IS_RGB;
    static constexpr bool rgbLed = ledPin == GPIO_NUM_NC ? false : IS_RGB;
    // NOLINTEND(bugprone-branch-clone,readability-simplify-boolean-expr)
    static constexpr EOrder ledColorChannelOrder = RGB_LED_COLOR_CHANNEL_ORDER;

    static constexpr unsigned char impulsesPerRevolution = IMPULSES_PER_REVOLUTION;
    static constexpr float flywheelInertia = FLYWHEEL_INERTIA;
    static constexpr unsigned short ledBlinkFrequency = LED_BLINK_FREQUENCY;
    static constexpr float sprocketRadius = SPROCKET_RADIUS / 100;
    static constexpr float concept2MagicNumber = CONCEPT_2_MAGIC_NUMBER;

    static constexpr precision angularDisplacementPerImpulse = (2 * PI) / impulsesPerRevolution;
    static constexpr unsigned char driveHandleForcesMaxCapacity = UCHAR_MAX;

    // Sensor signal filter settings
    static constexpr unsigned short rotationDebounceTimeMin = ROTATION_DEBOUNCE_TIME_MIN * 1'000;
    static constexpr unsigned int rowingStoppedThresholdPeriod = ROWING_STOPPED_THRESHOLD_PERIOD * 1'000;

    // Drag factor filter settings
    static constexpr float goodnessOfFitThreshold = GOODNESS_OF_FIT_THRESHOLD;
    static constexpr unsigned int maxDragFactorRecoveryPeriod = MAX_DRAG_FACTOR_RECOVERY_PERIOD * 1'000;
    static constexpr float lowerDragFactorThreshold = LOWER_DRAG_FACTOR_THRESHOLD / 1e6;
    static constexpr float upperDragFactorThreshold = UPPER_DRAG_FACTOR_THRESHOLD / 1e6;
    static constexpr unsigned char dragCoefficientsArrayLength = DRAG_COEFFICIENTS_ARRAY_LENGTH;

    // Stroke phase detection filter settings
    static constexpr StrokeDetectionType strokeDetectionType = STROKE_DETECTION;
    static constexpr float minimumPoweredTorque = MINIMUM_POWERED_TORQUE;
    static constexpr float minimumDragTorque = MINIMUM_DRAG_TORQUE;
    static constexpr float minimumRecoverySlopeMargin = MINIMUM_RECOVERY_SLOPE_MARGIN / 1e6;
    static constexpr float minimumRecoverySlope = MINIMUM_RECOVERY_SLOPE;
    static constexpr unsigned int minimumRecoveryTime = MINIMUM_RECOVERY_TIME * 1'000;
    static constexpr unsigned int minimumDriveTime = MINIMUM_DRIVE_TIME * 1'000;
    static constexpr unsigned char impulseDataArrayLength = IMPULSE_DATA_ARRAY_LENGTH;

    // Device power management settings
    static constexpr gpio_num_t batteryPinNumber = BATTERY_PIN_NUMBER;
    static constexpr unsigned char voltageDividerRatio = VOLTAGE_DIVIDER_RATIO;
    static constexpr float batteryVoltageMin = BATTERY_VOLTAGE_MIN / voltageDividerRatio;
    static constexpr float batteryVoltageMax = BATTERY_VOLTAGE_MAX / voltageDividerRatio;
    static constexpr unsigned char batteryLevelArrayLength = BATTERY_LEVEL_ARRAY_LENGTH;
    static constexpr unsigned char initialBatteryLevelMeasurementCount = INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT;
    static constexpr unsigned int batteryMeasurementFrequency = BATTERY_MEASUREMENT_FREQUENCY * 60 * 1'000;
    static constexpr unsigned int deepSleepTimeout = DEEP_SLEEP_TIMEOUT * 60 * 1'000;
};