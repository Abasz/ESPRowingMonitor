#include "ArduinoLog.h"

#include "EEPROM.service.h"

EEPROMService::EEPROMService(Preferences &_preferences) : preferences(_preferences)
{
}

// TODO: determine if the preferences.end() function call is necessary in my case
void EEPROMService::setup()
{
    Log.infoln("Setup EEPROM Data store");
    if (!preferences.begin("bikeSettings"))
    {
        Log.warningln("Error opening EEPROM using default bike settings");
    }

    if (!preferences.isKey(logLevelAddress))
    {
        Log.infoln("Setting LogLevel to default");
        preferences.putUChar(logLevelAddress, static_cast<byte>(Settings::defaultLogLevel));
    }

    logLevel = static_cast<ArduinoLogLevel>(preferences.getUChar(logLevelAddress, static_cast<byte>(Settings::defaultLogLevel)));

    Log.verboseln("%s: %d", logLevelAddress, logLevel);
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
}

ArduinoLogLevel EEPROMService::getLogLevel() const
{
    return logLevel;
}
