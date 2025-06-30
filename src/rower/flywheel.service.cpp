// #include <algorithm>

#include "ArduinoLog.h"

#include "globals.h"

#include "./flywheel.service.h"

// using std::minmax;

FlywheelService::FlywheelService() {}

void FlywheelService::setup(const RowerProfile::MachineSettings machineSettings, const RowerProfile::SensorSignalSettings sensorSignalSettings)
{
#if ENABLE_RUNTIME_SETTINGS
    angularDisplacementPerImpulse = (2 * PI) / machineSettings.impulsesPerRevolution;
    rotationDebounceTimeMin = sensorSignalSettings.rotationDebounceTimeMin;
#endif

    pinMode(Configurations::sensorPinNumber, INPUT_PULLUP);
    Log.verboseln("Attach interrupt");
    attachRotationInterrupt();
}

RowingDataModels::FlywheelData FlywheelService::getData()
{
    detachRotationInterrupt();
    isDataChanged = false;

    RowingDataModels::FlywheelData data = {
        .rawImpulseCount = impulseCount,
        .deltaTime = cleanDeltaTime,
        .totalTime = totalTime,
        .totalAngularDisplacement = totalAngularDisplacement,
        .cleanImpulseTime = lastCleanImpulseTime,
        // TODO: These serve debugging purposes, may be deleted
        .rawImpulseTime = lastRawImpulseTime,
    };
    attachRotationInterrupt();

    return data;
}

bool FlywheelService::hasDataChanged() const
{
    return isDataChanged;
}

void FlywheelService::processRotation(const unsigned long now)
{
    const auto currentRawImpulseDeltaTime = now - lastRawImpulseTime;

    if (currentRawImpulseDeltaTime < rotationDebounceTimeMin)
    {
        return;
    }

    isDataChanged = true;

    lastRawImpulseTime = now;

    const auto currentDeltaTime = now - lastCleanImpulseTime;

    // auto deltaTimeDiffPair = minmax<volatile unsigned long>(currentDeltaTime, lastDeltaTime);
    // auto deltaImpulseTimeDiff = deltaTimeDiffPair.second - deltaTimeDiffPair.first;

    // lastDeltaTime = currentDeltaTime;
    // // We disregard rotation signals that are non sensible (the absolute difference of the current and the previous deltas exceeds the current delta)
    // TODO: determine if it makes sense that the stroke engine updates the cyclePhase from outside or not. Question is whether this filtering method would really add considering the new algo
    // if (deltaImpulseTimeDiff > currentDeltaTime && cyclePhase == CyclePhase::Recovery)
    //     return;

    cleanDeltaTime = currentDeltaTime /* lastDeltaTime */;
    totalTime = totalTime + cleanDeltaTime;
    impulseCount = impulseCount + 1;
    lastCleanImpulseTime = now;
    totalAngularDisplacement = totalAngularDisplacement + angularDisplacementPerImpulse;
}