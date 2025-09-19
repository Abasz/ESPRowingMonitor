#pragma once

#include <climits>
#include <string>

#include "Arduino.h"
#include "ArduinoLog.h"
#include "FastLED.h"

#include "./macros.h"
#include "./settings.model.h"

using std::string;

class Configurations
{
public:
    typedef PRECISION precision;

    static constexpr unsigned char maxConnectionCount = 2;

    static constexpr bool logCalibration = LOG_CALIBRATION;
    static constexpr ArduinoLogLevel defaultLogLevel = DEFAULT_CPS_LOGGING_LEVEL;
    static constexpr bool supportSdCardLogging = SUPPORT_SD_CARD_LOGGING;

    static constexpr bool isRuntimeSettingsEnabled = ENABLE_RUNTIME_SETTINGS;
    static constexpr bool isDebounceFilterEnabled = ENABLE_DEBOUNCE_FILTER;

    // Bluetooth Settings
    static constexpr BleServiceFlag defaultBleServiceFlag = DEFAULT_BLE_SERVICE;
    static constexpr bool hasExtendedBleMetrics = HAS_BLE_EXTENDED_METRICS;
    static constexpr bool enableBluetoothDeltaTimeLogging = ENABLE_BLUETOOTH_DELTA_TIME_LOGGING;
    static constexpr BleSignalStrength bleSignalStrength = BLE_SIGNAL_STRENGTH;

    static constexpr bool addBleServiceStringToName = ADD_BLE_SERVICE_TO_DEVICE_NAME;
    static constexpr bool enableSerialInDeviceName = ADD_SERIAL_TO_DEVICE_NAME;
    inline static const string deviceName = TOSTRING(DEVICE_NAME);
    inline static const string modelNumber = TOSTRING(MODEL_NUMBER);
    // NOLINTNEXTLINE(readability-redundant-string-init)
    inline static const string serialNumber = SERIAL_NUMBER;
    inline static const string firmwareVersion = string(getCompileDate().data(), getCompileDate().size());
    inline static const string hardwareRevision = string(getHardwareRevision());

    // Hardware settings
    static constexpr BaudRates baudRate = BAUD_RATE;

    static constexpr gpio_num_t sdCardChipSelectPin = SD_CARD_CHIP_SELECT_PIN;
    static constexpr gpio_num_t sensorPinNumber = SENSOR_PIN_NUMBER;
    static constexpr gpio_num_t wakeupPinNumber = WAKEUP_SENSOR_PIN_NUMBER;
    static constexpr bool hasWakeupPinNumber = wakeupPinNumber != GPIO_NUM_NC;
    static constexpr gpio_num_t sensorOnSwitchPinNumber = SENSOR_ON_SWITCH_PIN_NUMBER;
    static constexpr bool hasSensorOnSwitchPinNumber = sensorOnSwitchPinNumber != GPIO_NUM_NC;

    static constexpr unsigned short ledBlinkFrequency = LED_BLINK_FREQUENCY;
    static constexpr gpio_num_t ledPin = static_cast<gpio_num_t>(LED_PIN);
    // NOLINTBEGIN(bugprone-branch-clone,readability-simplify-boolean-expr)
    static constexpr bool isRgb = ledPin == GPIO_NUM_NC ? false : IS_RGB;
    // NOLINTEND(bugprone-branch-clone,readability-simplify-boolean-expr)
    static constexpr EOrder ledColorChannelOrder = RGB_LED_COLOR_CHANNEL_ORDER;

    // Device power management settings
    static constexpr gpio_num_t batteryPinNumber = BATTERY_PIN_NUMBER;
    static constexpr unsigned char voltageDividerRatio = VOLTAGE_DIVIDER_RATIO;
    static constexpr float batteryVoltageMin = BATTERY_VOLTAGE_MIN / voltageDividerRatio;
    static constexpr float batteryVoltageMax = BATTERY_VOLTAGE_MAX / voltageDividerRatio;
    static constexpr unsigned char batteryLevelArrayLength = BATTERY_LEVEL_ARRAY_LENGTH;
    static constexpr unsigned char initialBatteryLevelMeasurementCount = INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT;
    static constexpr unsigned int batteryMeasurementFrequency = BATTERY_MEASUREMENT_FREQUENCY * 60 * 1'000;
    static constexpr unsigned int deepSleepTimeout = DEEP_SLEEP_TIMEOUT * 60 * 1'000;

    static constexpr unsigned short defaultAllocationCapacity = RowerProfile::Defaults::minimumRecoveryTime / RowerProfile::Defaults::rotationDebounceTimeMin;
};