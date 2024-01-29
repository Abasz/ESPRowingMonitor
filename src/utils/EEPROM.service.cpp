#include "ArduinoLog.h"

#include "EEPROM.service.h"

EEPROMService::EEPROMService(Preferences &_preferences) : preferences(_preferences)
{
}

// TODO: determine if the preferences.end() function call is necessary in my case
void EEPROMService::setup()
{
    Log.infoln("Setup EEPROM Data store");
    if (!preferences.begin("monitorSettings"))
    {
        Log.warningln("Error opening EEPROM using default ESP Rowing Monitor settings");
    }

    if (!preferences.isKey(logLevelAddress))
    {
        Log.infoln("Setting LogLevel to default");
        preferences.putUChar(logLevelAddress, static_cast<unsigned char>(Configurations::defaultLogLevel));
    }
    if (!preferences.isKey(bleServiceFlagAddress))
    {
        Log.infoln("Setting BleServiceFlag to default");
        preferences.putUChar(bleServiceFlagAddress, static_cast<unsigned char>(Configurations::defaultBleServiceFlag));
    }

    if constexpr (Configurations::enableWebSocketDeltaTimeLogging)
    {
        if (!preferences.isKey(websocketDeltaTimeLoggingAddress))
        {
            Log.infoln("Setting WebSocket deltaTime log location");
            preferences.putBool(websocketDeltaTimeLoggingAddress, Configurations::enableWebSocketDeltaTimeLogging);
        }
        logToWebsocket = preferences.getBool(websocketDeltaTimeLoggingAddress, Configurations::enableWebSocketDeltaTimeLogging);
        Log.verboseln("%s: %d", websocketDeltaTimeLoggingAddress, logToWebsocket);
    }

    logLevel = static_cast<ArduinoLogLevel>(preferences.getUChar(logLevelAddress, static_cast<unsigned char>(Configurations::defaultLogLevel)));
    bleServiceFlag = static_cast<BleServiceFlag>(preferences.getUChar(bleServiceFlagAddress, static_cast<unsigned char>(Configurations::defaultBleServiceFlag)));

    Log.verboseln("%s: %d", logLevelAddress, logLevel);
    Log.verboseln("%s: %d", bleServiceFlagAddress, bleServiceFlag);
}

void EEPROMService::setLogLevel(ArduinoLogLevel newLogLevel)
{
    int intLogLevel = static_cast<int>(newLogLevel);
    if (intLogLevel < 0 || intLogLevel > 6)
    {
        Log.errorln("Invalid LogLevel setting, should be between 0-6");
        return;
    }

    preferences.putUChar(logLevelAddress, intLogLevel);
    Log.setLevel(intLogLevel);
    logLevel = newLogLevel;
}

void EEPROMService::setLogToWebsocket(bool shouldLogToWebSocket)
{
    if constexpr (!Configurations::enableWebSocketDeltaTimeLogging)
    {
        Log.warningln("Not able to change deltaTime logging as the feature is disabled");

        return;
    }
    preferences.putBool(websocketDeltaTimeLoggingAddress, shouldLogToWebSocket);
    logToWebsocket = shouldLogToWebSocket;
}

void EEPROMService::setBleServiceFlag(BleServiceFlag newServiceFlag)
{
    int intBleServiceFlag = static_cast<int>(newServiceFlag);
    if (intBleServiceFlag < 0 || intBleServiceFlag > 1)
    {
        Log.errorln("Invalid BLE Service setting, should be between 0 or 1");
        return;
    }

    bleServiceFlag = newServiceFlag;
    preferences.putUChar(bleServiceFlagAddress, intBleServiceFlag);
}

BleServiceFlag EEPROMService::getBleServiceFlag() const
{
    return bleServiceFlag;
}

ArduinoLogLevel EEPROMService::getLogLevel() const
{
    return logLevel;
}

bool EEPROMService::getLogToWebsocket() const
{
    return logToWebsocket;
}
