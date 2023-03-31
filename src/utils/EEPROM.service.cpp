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
        preferences.putUChar(logLevelAddress, static_cast<unsigned char>(Settings::defaultLogLevel));
    }
    if (!preferences.isKey(logLevelAddress))
    {
        Log.infoln("Setting BleServiceFlag to default");
        preferences.putUChar(bleServiceFlagAddress, static_cast<unsigned char>(Settings::defaultBleServiceFlag));
    }

    logLevel = static_cast<ArduinoLogLevel>(preferences.getUChar(logLevelAddress, static_cast<unsigned char>(Settings::defaultLogLevel)));
    bleServiceFlag = static_cast<BleServiceFlag>(preferences.getUChar(bleServiceFlagAddress, static_cast<unsigned char>(Settings::defaultBleServiceFlag)));

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

void EEPROMService::setBleServiceFlag(BleServiceFlag newServiceFlag)
{
    int intBleServiceFlag = static_cast<int>(newServiceFlag);
    if (intBleServiceFlag < 0 || intBleServiceFlag > 1)
    {
        Log.errorln("Invalid BLE Service setting, should be between 0 or 1");
        return;
    }

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
