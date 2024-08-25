// NOLINTBEGIN
#pragma once

#include <cstdio>

#include "../../src/utils/configuration.h"

class Logging
{
public:
    template <class T, typename... Args>
    void infoln(T msg, Args... args)
    {
        if (Configurations::defaultLogLevel >= ArduinoLogLevel::LogLevelInfo)
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
        if (Configurations::defaultLogLevel >= ArduinoLogLevel::LogLevelVerbose)
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
        if (Configurations::defaultLogLevel >= ArduinoLogLevel::LogLevelTrace)
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
        if (Configurations::defaultLogLevel >= ArduinoLogLevel::LogLevelWarning)
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
        if (Configurations::defaultLogLevel >= ArduinoLogLevel::LogLevelError)
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