#pragma once

#include "../../rower/stroke.model.h"
#include "../enums.h"
#include "../settings.model.h"

class IEEPROMService
{
protected:
    ~IEEPROMService() = default;

public:
    virtual void setup() = 0;

    virtual void setLogLevel(ArduinoLogLevel newLogLevel) = 0;
    virtual void setLogToBluetooth(bool shouldLogToBluetooth) = 0;
    virtual void setLogToSdCard(bool shouldLogToSdCard) = 0;
    virtual void setBleServiceFlag(BleServiceFlag newServiceFlag) = 0;

    virtual void setMachineSettings(RowerProfile::MachineSettings newMachineSettings) = 0;
    virtual void setSensorSignalSettings(RowerProfile::SensorSignalSettings newSensorSignalSettings) = 0;
    virtual void setDragFactorSettings(RowerProfile::DragFactorSettings newDragFactorSettings) = 0;
    virtual void setStrokePhaseDetectionSettings(RowerProfile::StrokePhaseDetectionSettings newStrokePhaseDetectionSettings) = 0;

    virtual BleServiceFlag getBleServiceFlag() const = 0;
    virtual ArduinoLogLevel getLogLevel() const = 0;
    virtual bool getLogToBluetooth() const = 0;
    virtual bool getLogToSdCard() const = 0;

    virtual RowerProfile::MachineSettings getMachineSettings() const = 0;
    virtual RowerProfile::SensorSignalSettings getSensorSignalSettings() const = 0;
    virtual RowerProfile::DragFactorSettings getDragFactorSettings() const = 0;
    virtual RowerProfile::StrokePhaseDetectionSettings getStrokePhaseDetectionSettings() const = 0;

    virtual bool validateMachineSettings(const RowerProfile::MachineSettings &newMachineSettings) const = 0;
    virtual bool validateSensorSignalSettings(const RowerProfile::SensorSignalSettings &newSensorSignalSettings) const = 0;
    virtual bool validateDragFactorSettings(const RowerProfile::DragFactorSettings &newDragFactorSettings) const = 0;
    virtual bool validateStrokePhaseDetectionSettings(const RowerProfile::StrokePhaseDetectionSettings &newStrokePhaseDetectionSettings) const = 0;
};