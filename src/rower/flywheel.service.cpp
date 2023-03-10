// #include <algorithm>

#include "ArduinoLog.h"

#include "flywheel.service.h"
#include "globals.h"

// using std::minmax;

FlywheelService::FlywheelService() {}

void FlywheelService::setup() const
{
    Log.verboseln("Attach interrupt");
    attachRotationInterrupt();
}

StrokeModel::FlywheelData FlywheelService::getData()
{
    // execution time: 12 microsec
    // auto start = micros();
    detachRotationInterrupt();
    isDataChanged = false;

    StrokeModel::FlywheelData data = {
        .rawImpulseCount = impulseCount,
        .deltaTime = cleanDeltaTime,
        .totalTime = totalTime,
        .totalAngularDisplacement = totalAngularDisplacement,
        // TODO: These serve debugging purposes, my be deleted
        .cleanImpulseTime = lastCleanImpulseTime,
        .rawImpulseTime = lastRawImpulseTime};
    attachRotationInterrupt();
    // auto stop = micros();

    // Serial.print("getCscData: ");
    // Serial.println(stop - start);
    return data;
}

bool FlywheelService::hasDataChanged() const
{
    return isDataChanged;
}

void FlywheelService::processRotation(unsigned long now)
{
    auto currentRawImpulseDeltaTime = now - lastRawImpulseTime;

    if (currentRawImpulseDeltaTime < Settings::rotationDebounceTimeMin * 1000)
        return;

    isDataChanged = true;

    lastRawImpulseTime = now;

    auto currentDeltaTime = now - lastCleanImpulseTime;

    // auto deltaTimeDiffPair = minmax<volatile unsigned long>(currentDeltaTime, lastDeltaTime);
    // auto deltaImpulseTimeDiff = deltaTimeDiffPair.second - deltaTimeDiffPair.first;

    // lastDeltaTime = currentDeltaTime;
    // // We disregard rotation signals that are non sensible (the absolute difference of the current and the previous deltas exceeds the current delta)
    // TODO: determine if it makes sense that the stroke engine updates the cyclePhase from outsde or not. questeion is whether this filtering method would really add considering the new algo
    // if (deltaImpulseTimeDiff > currentDeltaTime && cyclePhase == CyclePhase::Recovery)
    //     return;

    cleanDeltaTime = currentDeltaTime /* lastDeltaTime */;
    totalTime += cleanDeltaTime;
    impulseCount = impulseCount + 1;
    lastCleanImpulseTime = now;
    totalAngularDisplacement = totalAngularDisplacement + angularDisplacementPerImpulse;
}