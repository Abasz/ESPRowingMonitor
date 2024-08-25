#pragma once

#include <array>
#include <string_view>

#if defined(UNIT_TEST)
    #include "../../test/unit/include/test.settings.h"
#elif defined(USE_CUSTOM_SETTINGS)
    #include "../custom.settings.h"
#else
    #include "../default.settings.h"
#endif
#include "./enums.h"

using std::string_view;

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
constexpr string_view getCompileMonth()
{
    constexpr auto month = string_view(__DATE__).substr(0, 3);
    if (month == "Jan")
    {
        return "01";
    }
    if (month == "Feb")
    {
        return "02";
    }
    if (month == "Mar")
    {
        return "03";
    }
    if (month == "Apr")
    {
        return "04";
    }
    if (month == "May")
    {
        return "05";
    }
    if (month == "Jun")
    {
        return "06";
    }
    if (month == "Jul")
    {
        return "07";
    }
    if (month == "Aug")
    {
        return "08";
    }
    if (month == "Sep")
    {
        return "09";
    }
    if (month == "Oct")
    {
        return "10";
    }
    if (month == "Nov")
    {
        return "11";
    }
    if (month == "Dec")
    {
        return "12";
    }

    return "00";
}

constexpr std::array<char, 8> getCompileDate()
{
    constexpr auto day = string_view(__DATE__).substr(4, 2);
    constexpr auto month = getCompileMonth();
    constexpr auto year = string_view(__DATE__).substr(7, 4);

    return {year[0], year[1], year[2], year[3], month[0], month[1], day[0], day[1]};
}

#define PRECISION_FLOAT 0
#define PRECISION_DOUBLE 1
#define STROKE_DETECTION_TORQUE 0
#define STROKE_DETECTION_SLOPE 1
#define STROKE_DETECTION_BOTH 2

#define CONCAT2(A, B) A##B
#define CONCAT2_DEFERRED(A, B) CONCAT2(A, B)
#define IF_0(true_case, false_case) false_case
#define IF_1(true_case, false_case) true_case
#define IF(condition, true_case, false_case) CONCAT2_DEFERRED(IF_, condition)(true_case, false_case)
#define VAL(str) #str
#define TOSTRING(str) VAL(str)
#define EMPTY(...) (true __VA_OPT__(&&false))

#if !defined(DEFAULT_CPS_LOGGING_LEVEL)
    #define DEFAULT_CPS_LOGGING_LEVEL ArduinoLogLevel::LogLevelTrace
#endif

#if !defined(DEFAULT_BLE_SERVICE)
    #define DEFAULT_BLE_SERVICE BleServiceFlag::CpsService
#endif

#if STROKE_DETECTION_TYPE == STROKE_DETECTION_BOTH
    #define STROKE_DETECTION StrokeDetectionType::Both
#elif STROKE_DETECTION_TYPE == STROKE_DETECTION_SLOPE
    #define STROKE_DETECTION StrokeDetectionType::Slope
#else
    #define STROKE_DETECTION StrokeDetectionType::Torque
#endif

#if !defined(LED_PIN)
    #if defined(LED_BUILTIN)
        #define LED_PIN LED_BUILTIN
    #else
        #define LED_PIN GPIO_NUM_NC
    #endif
#endif

#if !defined(RGB_LED_COLOR_CHANNEL_ORDER)
    #define RGB_LED_COLOR_CHANNEL_ORDER RGB
#endif

#if !defined(BATTERY_PIN_NUMBER)
    #define BATTERY_PIN_NUMBER GPIO_NUM_NC
#endif

#if !defined(BAUD_RATE)
    #define BAUD_RATE BaudRates::Baud115200
#endif

#if !defined(SERIAL_NUMBER)
    #define SERIAL_NUMBER "03172022/1"
#endif

#if !defined(SENSOR_ON_SWITCH_PIN_NUMBER)
    #define SENSOR_ON_SWITCH_PIN_NUMBER GPIO_NUM_NC
#endif

#if !defined(SD_CARD_CHIP_SELECT_PIN)
    #define SD_CARD_CHIP_SELECT_PIN GPIO_NUM_NC
#endif

#if !defined(WAKEUP_SENSOR_PIN_NUMBER)
    #define WAKEUP_SENSOR_PIN_NUMBER GPIO_NUM_NC
#endif

#if !defined(LOG_CALIBRATION)
    #define LOG_CALIBRATION false
#endif

#if !defined(HAS_BLE_EXTENDED_METRICS)
    #define HAS_BLE_EXTENDED_METRICS true
#endif
#if !defined(BLE_SIGNAL_STRENGTH)
    #define BLE_SIGNAL_STRENGTH BleSignalStrength::Default
#endif

#if !defined(ENABLE_BLUETOOTH_DELTA_TIME_LOGGING)
    #define ENABLE_BLUETOOTH_DELTA_TIME_LOGGING false
#endif

#if !defined(SUPPORT_SD_CARD_LOGGING)
    #define SUPPORT_SD_CARD_LOGGING false
#endif

// Sanity checks and validations
static_assert(SUPPORT_SD_CARD_LOGGING == false || (SUPPORT_SD_CARD_LOGGING == true && SD_CARD_CHIP_SELECT_PIN != GPIO_NUM_NC), "SD Card chip select pin is not provided. Please define 'SD_CARD_CHIP_SELECT_PIN' with the GPIO to which chip select is connected");

#if (MAX_DRAG_FACTOR_RECOVERY_PERIOD) / (ROTATION_DEBOUNCE_TIME_MIN) > 1'250
    #error "Theoretically the recovery may end up creating a vector with a max of 1250 data points (which amount in reality would depend on, among others, the drag) that would use up too much memory and crash the application. Please reduce the MAX_DRAG_FACTOR_RECOVERY_PERIOD property to avoid this. Please see docs for further information"
#elif (MAX_DRAG_FACTOR_RECOVERY_PERIOD) / (ROTATION_DEBOUNCE_TIME_MIN) > 1'000
    #warning "Theoretically the recovery may end up creating a vector with a max of 1000 data points (which amount reality would depend on, among others, the drag). This vector size would use up a considerable amount of memory and can cause instability and crash. Please consider reducing the MAX_DRAG_FACTOR_RECOVERY_PERIOD property to avoid this. Please see docs for further information"
#endif

#if defined(FLOATING_POINT_PRECISION) && FLOATING_POINT_PRECISION != PRECISION_DOUBLE && FLOATING_POINT_PRECISION != PRECISION_FLOAT
    #error "Invalid floating point precision setting"
#endif
#if IMPULSE_DATA_ARRAY_LENGTH < 3
    #error "IMPULSE_DATA_ARRAY_LENGTH should not be less than 3"
#endif
#if IMPULSE_DATA_ARRAY_LENGTH > 18
    #error "Using too many data points will increase loop execution time. It should not be more than 18"
#endif

#if IMPULSE_DATA_ARRAY_LENGTH < 14
    #if !defined(FLOATING_POINT_PRECISION)
        #define PRECISION double
    #else
        #define PRECISION IF(FLOATING_POINT_PRECISION, double, float)
    #endif
#endif
#if IMPULSE_DATA_ARRAY_LENGTH >= 14 && IMPULSE_DATA_ARRAY_LENGTH < 16
    #if !defined(FLOATING_POINT_PRECISION)
        #define PRECISION float
    #elif defined(FLOATING_POINT_PRECISION)
        #if FLOATING_POINT_PRECISION == PRECISION_DOUBLE
            #warning "Using too many data points (i.e. setting `IMPULSE_DATA_ARRAY_LENGTH` to a high number) will increase loop execution time. Using 15 and a precision of double would require around 3.4-4ms to complete all calculations. Hence impulses may be missed. So setting precision to float to save on execution time (but potentially loose some precision). Thus, consider changing precision from double to float. For further details please refer to [docs](docs/settings.md#impulse_data_array_length)"
        #endif
        #define PRECISION IF(FLOATING_POINT_PRECISION, double, float)
    #endif
#endif
#if IMPULSE_DATA_ARRAY_LENGTH >= 16 && IMPULSE_DATA_ARRAY_LENGTH < 18
    #if (FLOATING_POINT_PRECISION == PRECISION_DOUBLE)
        #warning "Using too many data points (i.e. setting `IMPULSE_DATA_ARRAY_LENGTH` to a high number) will increase loop execution time. Using 16 and a precision of double would require 3.9-4.7ms to complete all calculation. Hence impulses may be missed. So setting precision to float to save on execution time (but potentially loose some precision). For further details please refer to [docs](docs/settings.md#impulse_data_array_length)"
    #endif
    #define PRECISION float
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)