// NOLINTBEGIN
#pragma once

#include <cstdio>

#include "Arduino.h"

#include "./test.settings.h"

typedef void (*printfunction)(Print *, int);

enum class ArduinoLogLevel : unsigned char
{
    LogLevelSilent = 0,
    LogLevelFatal = 1,
    LogLevelError = 2,
    LogLevelWarning = 3,
    LogLevelInfo = 4,
    LogLevelNotice = 4, // Same as INFO, kept for backward compatibility
    LogLevelTrace = 5,
    LogLevelVerbose = 6
};

class Logging
{
public:
    template <class T, typename... Args>
    void infoln(T msg, Args... args)
    {
        if (DEFAULT_CPS_LOGGING_LEVEL >= ArduinoLogLevel::LogLevelInfo)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
            printf(msg, args...);
            printf("\n");
#pragma GCC diagnostic pop
        }
    };

    template <class T, typename... Args>
    void verboseln(T msg, Args... args)
    {
        if (DEFAULT_CPS_LOGGING_LEVEL >= ArduinoLogLevel::LogLevelVerbose)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
            printf(msg, args...);
            printf("\n");
#pragma GCC diagnostic pop
        }
    };

    template <class T, typename... Args>
    void traceln(T msg, Args... args)
    {
        if (DEFAULT_CPS_LOGGING_LEVEL >= ArduinoLogLevel::LogLevelTrace)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
            printf(msg, args...);
            printf("\n");
#pragma GCC diagnostic pop
        }
    };

    template <class T, typename... Args>
    void warningln(T msg, Args... args)
    {
        if (DEFAULT_CPS_LOGGING_LEVEL >= ArduinoLogLevel::LogLevelWarning)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
            printf(msg, args...);
            printf("\n");
#pragma GCC diagnostic pop
        }
    };

    template <class T, typename... Args>
    void errorln(T msg, Args... args)
    {
        if (DEFAULT_CPS_LOGGING_LEVEL >= ArduinoLogLevel::LogLevelError)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
            printf(msg, args...);
            printf("\n");
#pragma GCC diagnostic pop
        }
    };

    virtual inline void setLevel(ArduinoLogLevel level) {}
    inline void begin(ArduinoLogLevel level, Print *logOutput, bool showLevel = true) {}
    inline void setPrefix(printfunction f) {}
};

extern Logging Log;
// NOLINTEND