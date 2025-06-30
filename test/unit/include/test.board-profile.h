#pragma once

#include "../../../src/utils/enums.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)

// General settings
#define BAUD_RATE BaudRates::Baud115200

// LED settings
#define LED_BLINK_FREQUENCY 1'000
#define LED_PIN LED_BUILTIN // Use GPIO_NUM_NC if no led is available
#define IS_RGB true

// Hardware settings
#define SENSOR_PIN_NUMBER GPIO_NUM_26
#define SENSOR_ON_SWITCH_PIN_NUMBER GPIO_NUM_10 // Use GPIO_NUM_NC if no sensor switch is available
#define WAKEUP_SENSOR_PIN_NUMBER GPIO_NUM_20    // Use GPIO_NUM_NC if no separate wakeup pin is available
#define SD_CARD_CHIP_SELECT_PIN GPIO_NUM_5

// Device power management settings
#define BATTERY_PIN_NUMBER GPIO_NUM_34
#define VOLTAGE_DIVIDER_RATIO 2
#define BATTERY_VOLTAGE_MIN 3.3
#define BATTERY_VOLTAGE_MAX 4.00
#define BATTERY_LEVEL_ARRAY_LENGTH 5
#define INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT 10
#define BATTERY_MEASUREMENT_FREQUENCY 10
#define DEEP_SLEEP_TIMEOUT 4

// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-macro-to-enum)