#include "ArduinoLog.h"

#include "stroke.controller.h"

StrokeController::StrokeController(StrokeService &_strokeService) : strokeService(_strokeService)
{
}

void StrokeController::begin() const
{
    Log.infoln("Setting up stroke monitor controller");
    strokeService.setup();
}

void StrokeController::update()
{
    if (strokeService.hasDataChanged())
    {
        auto lastRawRevTime = cscData.rawImpulseTime;
        cscData = strokeService.getData();
        if (lastRawRevTime != cscData.rawImpulseTime)
            Log.verboseln("rawImpulseTime: %u", cscData.rawImpulseTime);

        if (cscData.rawDeltaImpulseTime != lastDeltaRead)
        {
            Log.traceln("deltaTime: %u", cscData.rawDeltaImpulseTime);
            Log.verboseln("cleanDeltaTime: %u", cscData.cleanDeltaImpulseTime);
            lastDeltaRead = cscData.rawDeltaImpulseTime;
        }
    }
}

unsigned long long StrokeController::getLastRevTime() const
{
    return cscData.lastRevTime;
}

unsigned int StrokeController::getRevCount() const
{
    return cscData.revCount;
}

unsigned long long StrokeController::getLastStrokeTime() const
{
    return cscData.lastStrokeTime;
}

unsigned short StrokeController::getStrokeCount() const
{
    return cscData.strokeCount;
}

unsigned long StrokeController::getRawImpulseTime() const
{
    return cscData.rawImpulseTime;
}

double StrokeController::getDriveDuration() const
{
    return cscData.driveDuration / 1e6;
}

double StrokeController::getRecoveryDuration() const
{
    return cscData.recoveryDuration / 1e6;
}

short StrokeController::getAvgStrokePower() const
{
    return lround(cscData.avgStrokePower);
}

unsigned int StrokeController::getDistance() const
{
    return round(cscData.distance);
}

byte StrokeController::getDragFactor() const
{
    return lround(cscData.dragCoefficient * 1e6);
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
    previousRevCount = cscData.revCount;
}

void StrokeController::setPreviousStrokeCount()
{
    previousStrokeCount = cscData.strokeCount;
}