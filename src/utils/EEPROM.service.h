#pragma once

#include "ArduinoLog.h"
#include "Preferences.h"

#include "./configuration.h"
#include "./enums.h"

class EEPROMService
{
    Preferences &preferences;

    static constexpr const char *logLevelAddress = "logLevel";
    static constexpr const char *websocketDeltaTimeLoggingAddress = "wsLogging";
    static constexpr const char *bluetoothDeltaTimeLoggingAddress = "bleLogging";
    static constexpr const char *sdCardLoggingAddress = "sdCardLogging";
    static constexpr const char *bleServiceFlagAddress = "bleService";

    ArduinoLogLevel logLevel = Configurations::defaultLogLevel;
    bool logToWebsocket = Configurations::enableWebSocketDeltaTimeLogging;
    bool logToBluetooth = Configurations::enableBluetoothDeltaTimeLogging;
    bool logToSdCard = false;
    BleServiceFlag bleServiceFlag = Configurations::defaultBleServiceFlag;

public:
    explicit EEPROMService(Preferences &_preferences);

    void setup();

    void setLogLevel(ArduinoLogLevel newLogLevel);
    void setLogToWebsocket(bool shouldLogToWebSocket);
    void setLogToBluetooth(bool shouldLogToBluetooth);
    void setLogToSdCard(bool shouldLogToSdCard);
    void setBleServiceFlag(BleServiceFlag newServiceFlag);

    BleServiceFlag getBleServiceFlag() const;
    ArduinoLogLevel getLogLevel() const;
    bool getLogToWebsocket() const;
    bool getLogToBluetooth() const;
    bool getLogToSdCard() const;
};