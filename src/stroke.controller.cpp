#include <Arduino.h>

#include "stroke.controller.h"

StrokeController::StrokeController() : strokeService(StrokeService())
{
}

void StrokeController::begin() const
{
    Serial.println("Setting up stroke monitor controller");
}

StrokeModel::CscData StrokeController::getCscData() const
{
    return strokeService.getCscData();
}

unsigned long StrokeController::getLastRevTime() const
{
    return strokeService.getLastRevTime();
}

unsigned long StrokeController::getLastRevReadTime() const
{
    return lastRevReadTime;
}

void StrokeController::setLastRevReadTime()
{

    lastRevReadTime = strokeService.getLastRevTime();
}

void StrokeController::processRotation(unsigned long now = 0)
{
    strokeService.processRotation(now);
}