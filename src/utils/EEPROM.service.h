#pragma once

#include "Preferences.h"

#include "ArduinoLog.h"

#include "configuration.h"
#include "enums.h"

class EEPROMService
{
    Preferences &preferences;

    static constexpr const char *logLevelAddress = "logLevel";
    static constexpr const char *websocketDeltaTimeLoggingAddress = "wsLogging";
    static constexpr const char *sdCardLoggingAddress = "sdCardLogging";
    static constexpr const char *bleServiceFlagAddress = "bleService";

    ArduinoLogLevel logLevel = Configurations::defaultLogLevel;
    bool logToWebsocket = Configurations::enableWebSocketDeltaTimeLogging;
    bool logToSdCard = false;
    BleServiceFlag bleServiceFlag = Configurations::defaultBleServiceFlag;

public:
    explicit EEPROMService(Preferences &_preferences);

    void setup();

    void setLogLevel(ArduinoLogLevel newLogLevel);
    void setLogToWebsocket(bool shouldLogToWebSocket);
    void setLogToSdCard(bool shouldLogToSdCard);
    void setBleServiceFlag(BleServiceFlag newServiceFlag);

    BleServiceFlag getBleServiceFlag() const;
    ArduinoLogLevel getLogLevel() const;
    bool getLogToWebsocket() const;
    bool getLogToSdCard() const;
};