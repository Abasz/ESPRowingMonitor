#pragma once

#include <Preferences.h>

#include "ArduinoLog.h"

#include "enums.h"
#include "settings.h"

class EEPROMService
{
    Preferences &preferences;

    static constexpr char const *logLevelAddress = "logLevel";
    static constexpr char const *bleServiceFlagAddress = "bleService";

    ArduinoLogLevel logLevel = Settings::defaultLogLevel;

public:
    EEPROMService(Preferences &_preferences);

    void setup();

    void setLogLevel(ArduinoLogLevel newLogLevel);

    ArduinoLogLevel getLogLevel() const;
};