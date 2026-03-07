#include <cmath>

#include "ArduinoLog.h"

#include "./stroke.controller.h"

#include "../utils/EEPROM/EEPROM.service.interface.h"
#include "../utils/configuration.h"
#include "../utils/macros.h"
#include "../utils/settings.model.h"
#include "./flywheel.service.interface.h"
#include "./stroke.model.h"
#include "./stroke.service.interface.h"

StrokeController::StrokeController(IStrokeService &_strokeService, IFlywheelService &_flywheelService, IEEPROMService &_eepromService)
    : strokeService(_strokeService),
      flywheelService(_flywheelService),
      eepromService(_eepromService)
{
}

void StrokeController::begin()
{
    Log.infoln("Setting up rowing monitor controller");

    const auto machineSettings = eepromService.getMachineSettings();
    const auto sensorSignalSettings = eepromService.getSensorSignalSettings();
    flywheelService.setup(machineSettings, sensorSignalSettings);
#if ENABLE_RUNTIME_SETTINGS
    strokeService.setup(machineSettings, sensorSignalSettings, eepromService.getDragFactorSettings(), eepromService.getStrokePhaseDetectionSettings());
#endif
}

void StrokeController::update()
{
    strokeService.processFilterBuffer();

    if (flywheelService.hasDataChanged())
    {
        const auto lastFlywheelData = flywheelData;
        flywheelData = flywheelService.getData();
        if (lastFlywheelData.rawImpulseTime != flywheelData.rawImpulseTime)
        {
            Log.verboseln("rawImpulseTime: %u", flywheelData.rawImpulseTime);
        }

        if (lastFlywheelData.rawImpulseCount == flywheelData.rawImpulseCount)
        {
            return;
        }

        Log.traceln("deltaTime: %u", flywheelData.deltaTime);

        strokeService.processData(flywheelData);
        rowerState = strokeService.getData();
    }
}

const RowingDataModels::RowingMetrics &StrokeController::getAllData() const
{
    return rowerState;
}

unsigned long long StrokeController::getLastRevTime() const
{
    return rowerState.lastRevTime;
}

unsigned int StrokeController::getRevCount() const
{
    return std::lround(rowerState.distance);
}

unsigned long long StrokeController::getLastStrokeTime() const
{
    return rowerState.lastStrokeTime;
}

unsigned short StrokeController::getStrokeCount() const
{
    return rowerState.strokeCount;
}

unsigned long StrokeController::getRawImpulseCount() const
{
    return flywheelData.rawImpulseCount;
}

unsigned long StrokeController::getLastImpulseTime() const
{
    return flywheelData.cleanImpulseTime;
}

unsigned long StrokeController::getDeltaTime() const
{
    return flywheelData.deltaTime;
}

Configurations::precision StrokeController::getDriveDuration() const
{
    return rowerState.driveDuration / 1e6;
}

Configurations::precision StrokeController::getRecoveryDuration() const
{
    return rowerState.recoveryDuration / 1e6;
}

short StrokeController::getAvgStrokePower() const
{
    return static_cast<short>(std::round(rowerState.avgStrokePower));
}

Configurations::precision StrokeController::getDistance() const
{
    return rowerState.distance;
}

unsigned short StrokeController::getDragFactor() const
{
    return std::lround(rowerState.dragCoefficient * 1e6);
}

unsigned int StrokeController::getPreviousRevCount() const
{
    return previousRevCount;
}

unsigned int StrokeController::getPreviousStrokeCount() const
{
    return previousStrokeCount;
}

unsigned long StrokeController::getPreviousRawImpulseCount() const
{
    return previousRawImpulseCount;
}

void StrokeController::setPreviousRevCount()
{
    previousRevCount = std::lround(rowerState.distance);
}

void StrokeController::setPreviousStrokeCount()
{
    previousStrokeCount = rowerState.strokeCount;
}

void StrokeController::setPreviousRawImpulseCount()
{
    previousRawImpulseCount = flywheelData.rawImpulseCount;
}
