#include <limits>
#include <utility>

#include "ArduinoLog.h"

#include "globals.h"

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

    if constexpr (Configurations::isRuntimeSettingsEnabled)
    {
        if (!preferences.isKey(flywheelInertiaAddress))
        {
            Log.infoln("Setting Flywheel Inertia to default");
            preferences.putFloat(flywheelInertiaAddress, Configurations::flywheelInertia);
        }

        if (!preferences.isKey(concept2MagicNumberAddress))
        {
            Log.infoln("Setting Magic Constant to default");
            preferences.putFloat(concept2MagicNumberAddress, Configurations::concept2MagicNumber);
        }

        flywheelInertia = preferences.getFloat(flywheelInertiaAddress, Configurations::flywheelInertia);
        concept2MagicNumber = preferences.getFloat(concept2MagicNumberAddress, Configurations::concept2MagicNumber);

        std::string inertiaFormatted{};
        const auto stringSize = 10U;
        inertiaFormatted.reserve(stringSize);
        // Workaround of ArduinoLog library float precision limitation and size issue with <format> header
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        snprintf(inertiaFormatted.data(), stringSize, "%.5f", flywheelInertia);

        Log.verboseln("%s: %s", flywheelInertiaAddress, inertiaFormatted.c_str());
        Log.verboseln("%s: %F", concept2MagicNumberAddress, concept2MagicNumber);
    }

    logLevel = ArduinoLogLevel{preferences.getUChar(logLevelAddress, std::to_underlying(Configurations::defaultLogLevel))};
    bleServiceFlag = BleServiceFlag{preferences.getUChar(bleServiceFlagAddress, std::to_underlying(Configurations::defaultBleServiceFlag))};

    Log.verboseln("%s: %d", logLevelAddress, logLevel);
    Log.verboseln("%s: %d", bleServiceFlagAddress, bleServiceFlag);

    Log.verboseln("Free NVS entries: %u", preferences.freeEntries());
}

void EEPROMService::setLogLevel(const ArduinoLogLevel newLogLevel)
{
    const int intLogLevel = std::to_underlying(newLogLevel);
    if (!isInBounds(intLogLevel, 0, 6))
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
    const unsigned int intBleServiceFlag = std::to_underlying(newServiceFlag);
    if (!isInBounds(intBleServiceFlag, 0U, 2U))
    {
        Log.errorln("Invalid BLE Service setting, should be between 0 or 2");

        return;
    }

    bleServiceFlag = newServiceFlag;
    preferences.putUChar(bleServiceFlagAddress, intBleServiceFlag);
}

void EEPROMService::setMachineSettings(const RowerProfile::MachineSettings newMachineSettings)
{
    if constexpr (!Configurations::isRuntimeSettingsEnabled)
    {
        Log.warningln("Not able to set machine settings as runtime settings is not enabled");

        return;
    }

    if (!isInBounds(newMachineSettings.concept2MagicNumber, 0.0F, std::numeric_limits<float>::max()))
    {
        Log.errorln("Invalid magic number, should be greater than 0");

        return;
    }

    if (!isInBounds(newMachineSettings.flywheelInertia, 0.0F, std::numeric_limits<float>::max()))
    {
        Log.errorln("Invalid flywheel inertia, should be greater than 0");

        return;
    }

    preferences.putFloat(flywheelInertiaAddress, newMachineSettings.flywheelInertia);
    preferences.putFloat(concept2MagicNumberAddress, newMachineSettings.concept2MagicNumber);
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

RowerProfile::MachineSettings EEPROMService::getMachineSettings() const
{
    return RowerProfile::MachineSettings{
        .flywheelInertia = flywheelInertia,
        .concept2MagicNumber = concept2MagicNumber,
    };
}