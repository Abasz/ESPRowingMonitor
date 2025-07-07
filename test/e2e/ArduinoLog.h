// NOLINTBEGIN
#pragma once

#include <cstdio>

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
        if (ArduinoLogLevel::LogLevelTrace >= ArduinoLogLevel::LogLevelInfo)
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
        if (ArduinoLogLevel::LogLevelTrace >= ArduinoLogLevel::LogLevelVerbose)
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
        if (ArduinoLogLevel::LogLevelTrace >= ArduinoLogLevel::LogLevelTrace)
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
        if (ArduinoLogLevel::LogLevelTrace >= ArduinoLogLevel::LogLevelWarning)
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
        if (ArduinoLogLevel::LogLevelTrace >= ArduinoLogLevel::LogLevelError)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
            printf(msg, args...);
            printf("\n");
#pragma GCC diagnostic pop
        }
    };

    inline void setLevel(int level) {}
};

extern Logging Log;
// NOLINTEND