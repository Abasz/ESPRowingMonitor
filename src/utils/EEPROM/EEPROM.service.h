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
    static constexpr const char *sprocketRadiusAddress = "sprocketRadius";
    static constexpr const char *impulsesPerRevolutionAddress = "impulsesPerRev";

    static constexpr const char *rotationDebounceAddress = "signalDebounce";
    static constexpr const char *rowingStoppedPeriodAddress = "rowingStopped";

    ArduinoLogLevel logLevel = Configurations::defaultLogLevel;
    bool logToBluetooth = Configurations::enableBluetoothDeltaTimeLogging;
    bool logToSdCard = false;
    BleServiceFlag bleServiceFlag = Configurations::defaultBleServiceFlag;

    float flywheelInertia = RowerProfile::Defaults::flywheelInertia;
    float concept2MagicNumber = RowerProfile::Defaults::concept2MagicNumber;
    float sprocketRadius = RowerProfile::Defaults::sprocketRadius;
    unsigned char impulsesPerRevolution = RowerProfile::Defaults::impulsesPerRevolution;

    unsigned short rotationDebounceTimeMin = RowerProfile::Defaults::rotationDebounceTimeMin;
    unsigned int rowingStoppedThresholdPeriod = RowerProfile::Defaults::rowingStoppedThresholdPeriod;

    void initializeBaseSettings();
    void initializeMachineSettings();
    void initializeSensorSignalSettings();

public:
    explicit EEPROMService(Preferences &_preferences);

    void setup() override;

    void setLogLevel(ArduinoLogLevel newLogLevel) override;
    void setLogToBluetooth(bool shouldLogToBluetooth) override;
    void setLogToSdCard(bool shouldLogToSdCard) override;
    void setBleServiceFlag(BleServiceFlag newServiceFlag) override;

    void setMachineSettings(RowerProfile::MachineSettings newMachineSettings) override;
    void setSensorSignalSettings(RowerProfile::SensorSignalSettings newSensorSignalSettings) override;

    BleServiceFlag getBleServiceFlag() const override;
    ArduinoLogLevel getLogLevel() const override;
    bool getLogToBluetooth() const override;
    bool getLogToSdCard() const override;

    RowerProfile::MachineSettings getMachineSettings() const override;
    RowerProfile::SensorSignalSettings getSensorSignalSettings() const override;

    static bool validateMachineSettings(const RowerProfile::MachineSettings &newMachineSettings);
    static bool validateSensorSignalSettings(const RowerProfile::SensorSignalSettings &newSensorSignalSettings);
};