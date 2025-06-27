#pragma once

#include "ArduinoLog.h"
#include "Preferences.h"

#include "../configuration.h"
#include "../enums.h"
#include "./EEPROM.service.interface.h"

class EEPROMService final : public IEEPROMService
{
    Preferences &preferences;

    static constexpr const char *logLevelAddress = "logLevel";
    static constexpr const char *bluetoothDeltaTimeLoggingAddress = "bleLogging";
    static constexpr const char *sdCardLoggingAddress = "sdCardLogging";
    static constexpr const char *bleServiceFlagAddress = "bleService";

    static constexpr const char *flywheelInertiaAddress = "flywheelInertia";
    static constexpr const char *concept2MagicNumberAddress = "magicNumber";

    ArduinoLogLevel logLevel = Configurations::defaultLogLevel;
    bool logToBluetooth = Configurations::enableBluetoothDeltaTimeLogging;
    bool logToSdCard = false;
    BleServiceFlag bleServiceFlag = Configurations::defaultBleServiceFlag;

    float flywheelInertia = Configurations::flywheelInertia;
    float concept2MagicNumber = Configurations::concept2MagicNumber;

    void initializeBaseSettings();
    void initializeMachineSettings();

public:
    explicit EEPROMService(Preferences &_preferences);

    void setup() override;

    void setLogLevel(ArduinoLogLevel newLogLevel) override;
    void setLogToBluetooth(bool shouldLogToBluetooth) override;
    void setLogToSdCard(bool shouldLogToSdCard) override;
    void setBleServiceFlag(BleServiceFlag newServiceFlag) override;

    void setMachineSettings(RowerProfile::MachineSettings newMachineSettings) override;

    BleServiceFlag getBleServiceFlag() const override;
    ArduinoLogLevel getLogLevel() const override;
    bool getLogToBluetooth() const override;
    bool getLogToSdCard() const override;

    RowerProfile::MachineSettings getMachineSettings() const override;

    static bool validateMachineSettings(const RowerProfile::MachineSettings &newMachineSettings);
};