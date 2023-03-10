#include <cmath>

#include "ArduinoLog.h"

#include "stroke.controller.h"

StrokeController::StrokeController(StrokeService &_strokeService, FlywheelService &_flywheelService) : strokeService(_strokeService), flywheelService(_flywheelService)
{
}

void StrokeController::begin() const
{
    Log.infoln("Setting up rowing monitor controller");
    flywheelService.setup();
}

void StrokeController::update()
{
    if (flywheelService.hasDataChanged())
    {
        auto lastFlywheelData = flywheelData;
        flywheelData = flywheelService.getData();
        if (lastFlywheelData.rawImpulseTime != flywheelData.rawImpulseTime)
            Log.verboseln("rawImpulseTime: %u", flywheelData.rawImpulseTime);

        if (lastFlywheelData.rawImpulseCount == flywheelData.rawImpulseCount)
            return;

        Log.traceln("deltaTime: %u", flywheelData.deltaTime);

        strokeService.processData(flywheelData);
        rowerState = strokeService.getData();
    }
}

unsigned long long StrokeController::getLastRevTime() const
{
    return rowerState.lastRevTime;
}

unsigned int StrokeController::getRevCount() const
{
    return lround(rowerState.distance);
}

unsigned long long StrokeController::getLastStrokeTime() const
{
    return rowerState.lastStrokeTime;
}

unsigned short StrokeController::getStrokeCount() const
{
    return rowerState.strokeCount;
}

unsigned long StrokeController::getRawImpulseTime() const
{
    return flywheelData.rawImpulseTime;
}

double StrokeController::getDriveDuration() const
{
    return rowerState.driveDuration / 1e6;
}

double StrokeController::getRecoveryDuration() const
{
    return rowerState.recoveryDuration / 1e6;
}

short StrokeController::getAvgStrokePower() const
{
    return rowerState.avgStrokePower;
}

double StrokeController::getDistance() const
{
    return rowerState.distance;
}

unsigned char StrokeController::getDragFactor() const
{
    return lround(rowerState.dragCoefficient * 1e6);
}

unsigned int StrokeController::getPreviousRevCount() const
{
    return previousRevCount;
}

unsigned int StrokeController::getPreviousStrokeCount() const
{
    return previousStrokeCount;
}

void StrokeController::setPreviousRevCount()
{
    previousRevCount = lround(rowerState.distance);
}

void StrokeController::setPreviousStrokeCount()
{
    previousStrokeCount = rowerState.strokeCount;
}