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

    static constexpr const char *goodnessOfFitAddress = "goodnessOfFit";
    static constexpr const char *maxDragFactorRecoveryPeriodAddress = "maxDragPeriod";
    static constexpr const char *lowerDragFactorThresholdAddress = "lowerDragFactor";
    static constexpr const char *upperDragFactorThresholdAddress = "upperDragFactor";
    static constexpr const char *dragCoefficientsArrayLengthAddress = "dragArraySize";

    static constexpr const char *strokeDetectionTypeAddress = "detectionType";
    static constexpr const char *minimumPoweredTorqueAddress = "poweredTorque";
    static constexpr const char *minimumDragTorqueAddress = "dragTorque";
    static constexpr const char *minimumRecoverySlopeMarginAddress = "slopeMargin";
    static constexpr const char *minimumRecoverySlopeAddress = "recoverySlope";
    static constexpr const char *minimumRecoveryTimeAddress = "recoveryTime";
    static constexpr const char *minimumDriveTimeAddress = "driveTime";
    static constexpr const char *impulseDataArrayLengthAddress = "impulseArray";
    static constexpr const char *driveHandleForcesMaxCapacityAddress = "forceCapacity";

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

    float goodnessOfFitThreshold = RowerProfile::Defaults::goodnessOfFitThreshold;
    unsigned int maxDragFactorRecoveryPeriod = RowerProfile::Defaults::maxDragFactorRecoveryPeriod;
    float lowerDragFactorThreshold = RowerProfile::Defaults::lowerDragFactorThreshold;
    float upperDragFactorThreshold = RowerProfile::Defaults::upperDragFactorThreshold;
    unsigned char dragCoefficientsArrayLength = RowerProfile::Defaults::dragCoefficientsArrayLength;

    StrokeDetectionType strokeDetectionType = RowerProfile::Defaults::strokeDetectionType;
    float minimumPoweredTorque = RowerProfile::Defaults::minimumPoweredTorque;
    float minimumDragTorque = RowerProfile::Defaults::minimumDragTorque;
    float minimumRecoverySlopeMargin = RowerProfile::Defaults::minimumRecoverySlopeMargin;
    float minimumRecoverySlope = RowerProfile::Defaults::minimumRecoverySlope;
    unsigned int minimumRecoveryTime = RowerProfile::Defaults::minimumRecoveryTime;
    unsigned int minimumDriveTime = RowerProfile::Defaults::minimumDriveTime;
    unsigned char impulseDataArrayLength = RowerProfile::Defaults::impulseDataArrayLength;
    unsigned char driveHandleForcesMaxCapacity = RowerProfile::Defaults::driveHandleForcesMaxCapacity;

    void initializeBaseSettings();
    void initializeMachineSettings();
    void initializeSensorSignalSettings();
    void initializeDragFactorSettings();
    void initializeStrokePhaseDetectionSettings();

public:
    explicit EEPROMService(Preferences &_preferences);

    void setup() override;

    void setLogLevel(ArduinoLogLevel newLogLevel) override;
    void setLogToBluetooth(bool shouldLogToBluetooth) override;
    void setLogToSdCard(bool shouldLogToSdCard) override;
    void setBleServiceFlag(BleServiceFlag newServiceFlag) override;

    void setMachineSettings(RowerProfile::MachineSettings newMachineSettings) override;
    void setSensorSignalSettings(RowerProfile::SensorSignalSettings newSensorSignalSettings) override;
    void setDragFactorSettings(RowerProfile::DragFactorSettings newDragFactorSettings) override;
    void setStrokePhaseDetectionSettings(RowerProfile::StrokePhaseDetectionSettings newStrokePhaseDetectionSettings) override;

    BleServiceFlag getBleServiceFlag() const override;
    ArduinoLogLevel getLogLevel() const override;
    bool getLogToBluetooth() const override;
    bool getLogToSdCard() const override;

    RowerProfile::MachineSettings getMachineSettings() const override;
    RowerProfile::SensorSignalSettings getSensorSignalSettings() const override;
    RowerProfile::DragFactorSettings getDragFactorSettings() const override;
    RowerProfile::StrokePhaseDetectionSettings getStrokePhaseDetectionSettings() const override;

    bool validateMachineSettings(const RowerProfile::MachineSettings &newMachineSettings) const override;
    bool validateSensorSignalSettings(const RowerProfile::SensorSignalSettings &newSensorSignalSettings) const override;
    bool validateDragFactorSettings(const RowerProfile::DragFactorSettings &newDragFactorSettings) const override;
    bool validateStrokePhaseDetectionSettings(const RowerProfile::StrokePhaseDetectionSettings &newStrokePhaseDetectionSettings) const override;
};