#include <array>

#include <Arduino.h>

#include "ArduinoLog.h"

#include "stroke.controller.h"

using std::array;
using std::partial_sort_copy;

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
    cscData = strokeService.getData();
    if (cscData.lastRevTime != lastRevReadTime)
    {
        Log.infoln("deltaTime: %u", cscData.deltaTime);
        // Serial.print("deltaTimeDiff: ");
        // Serial.println(data.deltaTimeDiff);
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

double StrokeController::getDragCoefficient() const
{
    array<double, StrokeModel::DRAG_COEFFICIENTS_ARRAY_LENGTH> sortedArray{};

    partial_sort_copy(cscData.dragCoefficients.cbegin(), cscData.dragCoefficients.cend(), sortedArray.begin(), sortedArray.end());

    return sortedArray[sortedArray.size() / 2];
}

unsigned long StrokeController::getLastRevReadTime() const
{
    return lastRevReadTime;
}

void StrokeController::setLastRevReadTime()
{
    lastRevReadTime = cscData.lastRevTime;
}