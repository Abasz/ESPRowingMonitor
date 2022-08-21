#include <algorithm>
#include <array>
#include <math.h>
#include <numeric>

#include <Arduino.h>

#include "ArduinoLog.h"

#ifdef ROWING_SIMULATION
    #include "../test/globals.h"
#else
    #include "globals.h"
#endif

#include "stroke.service.h"

using std::accumulate;
using std::any_of;
using std::array;
using std::minmax;
using std::partial_sort_copy;

using StrokeModel::CscData;

StrokeService::StrokeService(LinearRegressorService &_regressorService) : regressorService(_regressorService)
{
}

bool StrokeService::isFlywheelUnpowered() const
{
    byte numberOfAccelerations = 0;
    byte i = cleanDeltaTimes.size() - 1;
    while (i > 0)
    {
        if (cleanDeltaTimes[i] >= cleanDeltaTimes[i - 1] || cleanDeltaTimes[i - 1] - cleanDeltaTimes[i] < Settings::maxDecelerationDeltaForPowered)
        {
            // Oldest interval (dataPoints[i]) is larger than the younger one (dataPoint[i-1], as the distance is
            // fixed, we are accelerating
            numberOfAccelerations++;
        }
        i--;
    }

    // If not all of the data points are consistently increasing or they are actually decreasing report unpowered
    if (numberOfAccelerations <= Settings::flywheelPowerChangeDetectionErrorThreshold)
    {
        return true;
    }

    return false;
}

bool StrokeService::isFlywheelPowered() const
{
    byte numberOfDecelerations = 0;
    byte i = cleanDeltaTimes.size() - 1;
    while (i > 0)
    {
        if (cleanDeltaTimes[i] < cleanDeltaTimes[i - 1] && cleanDeltaTimes[i - 1] - cleanDeltaTimes[i] > Settings::minDecelerationDeltaForUnpowered)
        {
            // Oldest interval (dataPoints[i]) is shorter than the younger one (dataPoint[i-1], as the distance is fixed, we
            // discovered a deceleration
            numberOfDecelerations++;
        }
        i--;
    }
    // If not all of the data points are consistently decreasing or they are actually increasing report powered
    if (numberOfDecelerations <= Settings::flywheelPowerChangeDetectionErrorThreshold)
    {
        return true;
    }

    return false;
}

void StrokeService::setup() const
{
    pinMode(Settings::sensorPinNumber, INPUT_PULLUP);
    Log.traceln("Attach interrupt");
    attachRotationInterrupt();
}

void StrokeService::calculateDragCoefficient()
{
    if (cleanDeltaTimes[Settings::deltaTimeArrayLength - 1] < Settings::dragFactorRotationDeltaUpperThreshold * 1000 || recoveryDuration > Settings::maxDragFactorRecoveryPeriod * 1000)
        return;

    if (regressorService.goodnessOfFit() < Settings::goodnessOfFitThreshold)
        return;

    auto rawNewDragCoefficient = (regressorService.slope() * Settings::flywheelInertia) / angularDisplacementPerImpulse;

    if (rawNewDragCoefficient > Settings::upperDragFactorThreshold ||
        rawNewDragCoefficient < Settings::lowerDragFactorThreshold)
        return;

    if (Settings::dragCoefficientsArrayLength > 1)
    {
        char i = Settings::dragCoefficientsArrayLength - 1;
        while (i > 0)
        {
            dragCoefficients[i] = dragCoefficients[i - 1];
            i--;
        }
        dragCoefficients[0] = rawNewDragCoefficient;

        array<double, Settings::dragCoefficientsArrayLength> sortedArray{};

        partial_sort_copy(dragCoefficients.cbegin(), dragCoefficients.cend(), sortedArray.begin(), sortedArray.end());
        rawNewDragCoefficient = sortedArray[sortedArray.size() / 2];
    }

    dragCoefficient = rawNewDragCoefficient;
}

void StrokeService::calculateAvgStrokePower()
{
    avgStrokePower = dragCoefficient * pow((impulseCount - strokeCycleStartIndex - 1 - driveStartImpulseCount) * angularDisplacementPerImpulse / ((lastDriveDuration + recoveryDuration) / 1e6), 3);
}

CscData StrokeService::getData() const
{
    // execution time: 12 microsec
    // auto start = micros();
    detachRotationInterrupt();
    CscData data = {
        .lastRevTime = lastRevTime,
        .revCount = revCount,
        .lastStrokeTime = lastStrokeTime,
        .strokeCount = strokeCount,
        .deltaTime = cleanDeltaTimes[0],
        .rawRevTime = previousRawRevTime,
        .driveDuration = lastDriveDuration,
        .distance = distance,
        .avgStrokePower = avgStrokePower,
        .dragCoefficient = dragCoefficient};
    attachRotationInterrupt();
    // auto stop = micros();

    // Serial.print("getCscData: ");
    // Serial.println(stop - start);
    return data;
}

bool StrokeService::hasDataChanged()
{
    if (previousRawRevTime != lastDataReadTime)
    {
        lastDataReadTime = previousRawRevTime;

        return true;
    }

    return false;
}

void StrokeService::processRotation(unsigned long now)
{
    auto currentRawDeltaTime = now - previousCleanRevTime;

    previousRawRevTime = now;

    if (currentRawDeltaTime < Settings::rotationDebounceTimeMin * 1000)
        return;

    auto currentCleanDeltaTime = now - previousCleanRevTime;

    auto deltaTimeDiffPair = minmax<volatile unsigned long>(currentCleanDeltaTime, previousDeltaTime);
    auto deltaTimeDiff = deltaTimeDiffPair.second - deltaTimeDiffPair.first;

    previousDeltaTime = currentCleanDeltaTime;
    // We disregard rotation signals that are non sensible (the absolute difference of the current and the previous deltas exceeds the current delta)
    // TODO: check if adding a constraint to being in the Recovery Phase would help with the data skipping in the drive phase where sudden high power if applied to the flywheel causing quick increase in the revtime. This needs to be tested with a low ROTATION_DEBOUNCE_TIME_MIN
    if (deltaTimeDiff > currentCleanDeltaTime /* && cyclePhase != CyclePhase::Drive */)
        return;

    // If we got this far, we must have a sensible delta for flywheel rotation time, updating the deltaTime array
    byte i = Settings::deltaTimeArrayLength - 1;
    while (i > 0)
    {
        cleanDeltaTimes[i] = cleanDeltaTimes[i - 1];
        rawDeltaTimes[i] = rawDeltaTimes[i - 1];
        i--;
    }

    rawDeltaTimes[0] = currentCleanDeltaTime;
    cleanDeltaTimes[0] = any_of(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cbegin() + Settings::rotationSmoothingFactor, [](unsigned long cleanDeltaTime)
                                { return cleanDeltaTime == 0; })
                             ? currentCleanDeltaTime
                             : lround((currentCleanDeltaTime + accumulate(cleanDeltaTimes.begin(), cleanDeltaTimes.begin() + Settings::rotationSmoothingFactor, 0)) / (Settings::rotationSmoothingFactor + 1.0));

    previousCleanRevTime = now;

    // If rotation delta exceeds the max debounce time and we are in Recovery Phase, the rower must have stopped. Setting cyclePhase to "Stopped"
    if (cyclePhase == CyclePhase::Recovery && recoveryDuration > Settings::rowingStoppedThresholdPeriod * 1000)
    {
        cyclePhase = CyclePhase::Stopped;

        driveDuration = 0;
        recoveryDuration = 0;
        avgStrokePower = 0;
        impulseCount = 0;

        return;
    }

    if (cyclePhase == CyclePhase::Stopped)
    {
        // We are currently in the "Stopped" phase, as power was not applied for a long period of time or the device just started. Since rotation was detected we check if cleanDeltaTimes array is filled (i.e. whether we have sufficient data for determining the next phase) and whether power is being applied to the flywheel
        if (
            any_of(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cend(), [](unsigned long cleanDeltaTime)
                   { return cleanDeltaTime == 0; }) ||
            !isFlywheelPowered())
        {
            return;
        }
        // Since we detected power, setting to "Drive" phase and increasing rotation count and registering rotation time
        cyclePhase = CyclePhase::Drive;
        driveStartTime = now - accumulate(rawDeltaTimes.cbegin(), rawDeltaTimes.cend() - Settings::flywheelPowerChangeDetectionErrorThreshold - 1, 0);
        driveStartImpulseCount = 0;
        regressorService.resetData();

        impulseCount = strokeCycleStartIndex + 1;
        auto startupRevCount = floor(impulseCount / Settings::impulsesPerRevolution);
        if (startupRevCount > 0)
        {
            revCount += startupRevCount;
            distance = round(pow((dragCoefficient * 1e6) / 2.8, 1 / 3.0) * (2.0 * PI) * revCount);
            lastRevTime = now;
        }

        return;
    }

    impulseCount++;
    if (impulseCount % Settings::impulsesPerRevolution == 0)
    {
        revCount++;
        distance = round(pow((dragCoefficient * 1e6) / 2.8, 1 / 3.0) * (2.0 * PI) * revCount);
        lastRevTime = now;
    }

    // we implement a finite state machine that goes between "Drive" and "Recovery" phases while paddling on the machine. This allows a phase-change if sufficient time has passed and there is a plausible flank
    if (cyclePhase == CyclePhase::Drive)
    {
        // We are currently in the "Drive" phase, lets determine what the next phase is (if we come from "Stopped" phase )
        if (isFlywheelUnpowered())
        {
            driveDuration += rawDeltaTimes[strokeCycleStartIndex];
            // It seems that we lost power to the flywheel lets check if drive time was sufficient for detecting a stroke (i.e. drivePhaseDuration exceeds debounce time)
            if (driveDuration > Settings::strokeDebounceTime * 1000)
            {
                // Here we can conclude the "Drive" phase as there is no more drive detected to the flywheel (e.g. for calculating power etc.)
                strokeCount++;
                lastDriveDuration = driveDuration;
                lastStrokeTime = now;
            }

            cyclePhase = CyclePhase::Recovery;
            driveDuration = 0;
            recoveryStartTime = now - accumulate(rawDeltaTimes.cbegin(), rawDeltaTimes.cend() - Settings::flywheelPowerChangeDetectionErrorThreshold - 1, 0);

            return;
        }

        driveDuration += rawDeltaTimes[strokeCycleStartIndex];

        return;
    }

    if (cyclePhase == CyclePhase::Recovery)
    {
        // We are currently in the "Recovery" phase, lets determine what the next phase is
        if (isFlywheelPowered())
        {
            regressorService.addToDataset(rawDeltaTimes[strokeCycleStartIndex]);
            recoveryDuration += rawDeltaTimes[strokeCycleStartIndex];
            // Here we can conclude the "Recovery" phase (and the current stroke cycle) as drive to the flywheel is detected (e.g. calculating drag factor)
            calculateDragCoefficient();
            calculateAvgStrokePower();

            cyclePhase = CyclePhase::Drive;
            driveStartTime = now - accumulate(rawDeltaTimes.cbegin(), rawDeltaTimes.cend() - Settings::flywheelPowerChangeDetectionErrorThreshold - 1, 0);
            driveStartImpulseCount = impulseCount - strokeCycleStartIndex - 1;
            recoveryDuration = 0;
            regressorService.resetData();

            return;
        }

        regressorService.addToDataset(rawDeltaTimes[strokeCycleStartIndex]);
        recoveryDuration += rawDeltaTimes[strokeCycleStartIndex];

        return;
    }
}