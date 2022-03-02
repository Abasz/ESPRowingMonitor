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
        cscData = strokeService.getData();
        Log.infoln("deltaTime: %u", cscData.deltaTime);
    }
}

unsigned long StrokeController::getLastRevTime() const
{
    return cscData.lastRevTime;
}

unsigned int StrokeController::getRevCount() const
{
    return cscData.revCount;
}

unsigned long StrokeController::getLastStrokeTime() const
{
    return cscData.lastStrokeTime;
}

unsigned short StrokeController::getStrokeCount() const
{
    return cscData.strokeCount;
}

unsigned int StrokeController::getDeltaTime() const
{
    return cscData.deltaTime;
}

double StrokeController::getDriveDuration() const
{
    return cscData.driveDuration / 1e6;
}

unsigned int StrokeController::getAvgStrokePower() const
{
    return lround(cscData.avgStrokePower);
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

unsigned int StrokeController::getPreviousAvgStrokePower() const
{
    return previousAvgStrokePower;
}

void StrokeController::setPreviousRevCount()
{
    previousRevCount = cscData.revCount;
}

void StrokeController::setPreviousStrokeCount()
{
    previousStrokeCount = cscData.strokeCount;
}

void StrokeController::setPreviousAvgStrokePower()
{
    previousAvgStrokePower = lround(cscData.avgStrokePower);
}