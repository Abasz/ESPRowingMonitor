#pragma once

#include "Preferences.h"

#include "ArduinoLog.h"

#include "../settings.h"
#include "enums.h"

class EEPROMService
{
    Preferences &preferences;

    static constexpr char const *logLevelAddress = "logLevel";
    static constexpr char const *bleServiceFlagAddress = "bleService";

    ArduinoLogLevel logLevel = Settings::defaultLogLevel;
    BleServiceFlag bleServiceFlag = Settings::defaultBleServiceFlag;

public:
    explicit EEPROMService(Preferences &_preferences);

    void setup();

    void setLogLevel(ArduinoLogLevel newLogLevel);
    void setBleServiceFlag(BleServiceFlag newServiceFlag);

    BleServiceFlag getBleServiceFlag() const;
    ArduinoLogLevel getLogLevel() const;
};