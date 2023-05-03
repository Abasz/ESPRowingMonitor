#pragma once

#ifdef UNIT_TEST
    #include "../../test/unit/test-settings.h"
#else
    #include "../settings.h"
#endif
#include "enums.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
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

#if STROKE_DETECTION_TYPE == STROKE_DETECTION_BOTH
    #define STROKE_DETECTION StrokeDetectionType::Both
#elif STROKE_DETECTION_TYPE == STROKE_DETECTION_SLOPE
    #define STROKE_DETECTION StrokeDetectionType::Slope
#else
    #define STROKE_DETECTION StrokeDetectionType::Torque
#endif

// Sanity checks and validations

#if defined(FLOATING_POINT_PRECISION) && FLOATING_POINT_PRECISION != PRECISION_DOUBLE && FLOATING_POINT_PRECISION != PRECISION_FLOAT
    #error "Invalid floating point precision setting"
#endif
#if IMPULSE_DATA_ARRAY_LENGTH < 3
    #error "IMPULSE_DATA_ARRAY_LENGTH should not be less than 3"
#endif
#if IMPULSE_DATA_ARRAY_LENGTH >= 18
    #error "Using too many data points will increase loop execution time. It should not be more than 17"
#endif
#if (!defined(LOCAL_SSID) || !defined(PASSPHRASE))
    #undef ENABLE_WEBSOCKET_MONITOR
    #define ENABLE_WEBSOCKET_MONITOR false
    #warning "Not provided SSID and/or Passphrase, disabling WebSocket monitor"
#endif

#if IMPULSE_DATA_ARRAY_LENGTH < 12
    #if !defined(FLOATING_POINT_PRECISION)
        #define PRECISION double
    #else
        #define PRECISION IF(FLOATING_POINT_PRECISION, double, float)
    #endif
#endif
#if IMPULSE_DATA_ARRAY_LENGTH >= 12 && IMPULSE_DATA_ARRAY_LENGTH < 15
    #if !defined(FLOATING_POINT_PRECISION)
        #define PRECISION float
    #elif defined(FLOATING_POINT_PRECISION)
        #if FLOATING_POINT_PRECISION == PRECISION_DOUBLE
            #warning "Using too many data points (i.e. setting `IMPULSE_DATA_ARRAY_LENGTH` to a high number) will increase loop execution time. Using 14 and a precision of double would require around 7ms to complete all calculations. Hence impulses may be missed. So setting precision to float to save on execution time (but potentially loose some precision). Thus, consider changing precision from double to float. For further details please refer to [docs](docs/settings.md#impulse_data_array_length)"
        #endif
        #define PRECISION IF(FLOATING_POINT_PRECISION, double, float)
    #endif
#endif
#if IMPULSE_DATA_ARRAY_LENGTH >= 15 && IMPULSE_DATA_ARRAY_LENGTH < 18
    #if (FLOATING_POINT_PRECISION == PRECISION_DOUBLE)
        #warning "Using too many data points (i.e. setting `IMPULSE_DATA_ARRAY_LENGTH` to a high number) will increase loop execution time. Using 15 and a precision of double would require more than 7ms to complete all calculation. Hence impulses may be missed. So setting precision to float to save on execution time (but potentially loose some precision). For further details please refer to [docs](docs/settings.md#impulse_data_array_length)"
    #endif
    #define PRECISION float
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)