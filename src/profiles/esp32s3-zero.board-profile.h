#pragma once

#include "../utils/enums.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)

// General settings
#define BAUD_RATE BaudRates::Baud1500000
#define BLE_SIGNAL_STRENGTH BleSignalStrength::MaxPower
#define SUPPORT_SD_CARD_LOGGING false

// LED settings
#define LED_BLINK_FREQUENCY 1'000
#define LED_PIN WS_RGB // Use GPIO_NUM_NC if no led is available
#define IS_RGB true

// Hardware settings
#define SENSOR_PIN_NUMBER GPIO_NUM_1
#define SENSOR_ON_SWITCH_PIN_NUMBER GPIO_NUM_2 // Use GPIO_NUM_NC if no sensor switch is available
#define WAKEUP_SENSOR_PIN_NUMBER GPIO_NUM_NC   // Use GPIO_NUM_NC if no separate wakeup pin is available

// Device power management settings
#define BATTERY_PIN_NUMBER GPIO_NUM_13
#define VOLTAGE_DIVIDER_RATIO 2
#define BATTERY_VOLTAGE_MIN 3.8
#define BATTERY_VOLTAGE_MAX 6.1
#define BATTERY_LEVEL_ARRAY_LENGTH 5
#define INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT 10
#define BATTERY_MEASUREMENT_FREQUENCY 10
#define DEEP_SLEEP_TIMEOUT 4

// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)