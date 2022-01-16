#include <array>

#include <Arduino.h>

#include "stroke.controller.h"

using std::array;
using std::partial_sort_copy;

StrokeController::StrokeController() : strokeService(StrokeService())
{
}

void StrokeController::begin() const
{
    strokeService.setup();
    Serial.println("Setting up stroke monitor controller");
}

void StrokeController::readCscData()
{
    cscData = strokeService.getData();
    if (cscData.lastRevTime != lastRevReadTime)
    {
        // Serial.print("deltaTime: ");
        Serial.println(cscData.deltaTime);
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

    partial_sort_copy(cscData.dragCoefficients.cbegin(), cscData.dragCoefficients.end(), sortedArray.begin(), sortedArray.end());

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

void StrokeController::processRotation(unsigned long now = 0)
{
    strokeService.processRotation(now);
}