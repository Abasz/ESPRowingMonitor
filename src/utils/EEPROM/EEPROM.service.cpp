#include <utility>

#include "ArduinoLog.h"

#include "./EEPROM.service.h"

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
        preferences.putUChar(logLevelAddress, std::to_underlying(Configurations::defaultLogLevel));
    }
    if (!preferences.isKey(bleServiceFlagAddress))
    {
        Log.infoln("Setting BleServiceFlag to default");
        preferences.putUChar(bleServiceFlagAddress, std::to_underlying(Configurations::defaultBleServiceFlag));
    }

    if constexpr (Configurations::enableBluetoothDeltaTimeLogging)
    {
        if (!preferences.isKey(bluetoothDeltaTimeLoggingAddress))
        {
            Log.infoln("Setting Bluetooth deltaTime log location");
            preferences.putBool(bluetoothDeltaTimeLoggingAddress, Configurations::enableBluetoothDeltaTimeLogging);
        }
        logToBluetooth = preferences.getBool(bluetoothDeltaTimeLoggingAddress, Configurations::enableBluetoothDeltaTimeLogging);
        Log.verboseln("%s: %d", bluetoothDeltaTimeLoggingAddress, logToBluetooth);
    }

    if constexpr (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC)
    {
        if (!preferences.isKey(sdCardLoggingAddress))
        {
            Log.infoln("Setting Sd Card log location");
            preferences.putBool(sdCardLoggingAddress, false);
        }
        logToSdCard = preferences.getBool(sdCardLoggingAddress, false);
        Log.verboseln("%s: %d", sdCardLoggingAddress, logToSdCard);
    }

    logLevel = ArduinoLogLevel{preferences.getUChar(logLevelAddress, std::to_underlying(Configurations::defaultLogLevel))};
    bleServiceFlag = BleServiceFlag{preferences.getUChar(bleServiceFlagAddress, std::to_underlying(Configurations::defaultBleServiceFlag))};

    Log.verboseln("%s: %d", logLevelAddress, logLevel);
    Log.verboseln("%s: %d", bleServiceFlagAddress, bleServiceFlag);
}

void EEPROMService::setLogLevel(const ArduinoLogLevel newLogLevel)
{
    const int intLogLevel = std::to_underlying(newLogLevel);
    if (intLogLevel < 0 || intLogLevel > 6)
    {
        Log.errorln("Invalid LogLevel setting, should be between 0-6");
        return;
    }

    preferences.putUChar(logLevelAddress, intLogLevel);
    Log.setLevel(intLogLevel);
    logLevel = newLogLevel;
}

void EEPROMService::setLogToBluetooth(const bool shouldLogToBluetooth)
{
    if constexpr (!Configurations::enableBluetoothDeltaTimeLogging)
    {
        Log.warningln("Not able to change Bluetooth deltaTime logging as the feature is disabled");

        return;
    }
    preferences.putBool(bluetoothDeltaTimeLoggingAddress, shouldLogToBluetooth);
    logToBluetooth = shouldLogToBluetooth;
}

void EEPROMService::setLogToSdCard(const bool shouldLogToSdCard)
{
    if constexpr (!Configurations::supportSdCardLogging)
    {
        Log.warningln("Not able to change Sd card logging as the feature is disabled");

        return;
    }

    if constexpr (Configurations::sdCardChipSelectPin == GPIO_NUM_NC)
    {
        Log.warningln("Not able to change Sd card logging as the chip select pin is not provided");

        return;
    }

    preferences.putBool(sdCardLoggingAddress, shouldLogToSdCard);
    logToSdCard = shouldLogToSdCard;
}

void EEPROMService::setBleServiceFlag(const BleServiceFlag newServiceFlag)
{
    const auto intBleServiceFlag = std::to_underlying(newServiceFlag);
    if (intBleServiceFlag < 0 || intBleServiceFlag > 2)
    {
        Log.errorln("Invalid BLE Service setting, should be between 0 or 2");
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

bool EEPROMService::getLogToBluetooth() const
{
    return logToBluetooth;
}

bool EEPROMService::getLogToSdCard() const
{
    return logToSdCard;
}