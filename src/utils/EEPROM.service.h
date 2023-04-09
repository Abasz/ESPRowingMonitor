#pragma once

#include "Preferences.h"

#include "ArduinoLog.h"

#include "configuration.h"
#include "enums.h"

class EEPROMService
{
    Preferences &preferences;

    static constexpr char const *logLevelAddress = "logLevel";
    static constexpr char const *bleServiceFlagAddress = "bleService";

    ArduinoLogLevel logLevel = Configurations::defaultLogLevel;
    BleServiceFlag bleServiceFlag = Configurations::defaultBleServiceFlag;

public:
    explicit EEPROMService(Preferences &_preferences);

    void setup();

    void setLogLevel(ArduinoLogLevel newLogLevel);
    void setBleServiceFlag(BleServiceFlag newServiceFlag);

    BleServiceFlag getBleServiceFlag() const;
    ArduinoLogLevel getLogLevel() const;
};