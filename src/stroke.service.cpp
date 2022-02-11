#include <array>
#include <algorithm>
#include <numeric>
#include <math.h>

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

StrokeService::StrokeService()
{
}

bool StrokeService::isFlywheelUnpowered() const
{
    byte numberOfAccelerations = 0;
    byte i = cleanDeltaTimes.size() - 1;
    while (i > 0)
    {
        if (cleanDeltaTimes[i] >= cleanDeltaTimes[i - 1])
        {
            // Oldest interval (dataPoints[i]) is larger than the younger one (datapoint[i-1], as the distance is
            // fixed, we are accelerating
            numberOfAccelerations++;
        }
        i--;
    }

    // If not all of the data points are consistently increasing or they are actually decreasing report unpowered
    if (numberOfAccelerations >= FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD)
    {
        return false;
    }

    return true;
}

bool StrokeService::isFlywheelPowered() const
{
    byte numberOfDecelerations = 0;
    byte i = cleanDeltaTimes.size() - 1;
    while (i > 0)
    {
        if (cleanDeltaTimes[i] < cleanDeltaTimes[i - 1])
        {
            // Oldest interval (dataPoints[i]) is shorter than the younger one (datapoint[i-1], as the distance is fixed, we
            // discovered a deceleration
            numberOfDecelerations++;
        }
        i--;
    }
    // If not all of the data points are consistently decreasing or they are actually increasing report powered
    if (numberOfDecelerations >= FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD)
    {
        return false;
    }

    return true;
}

void StrokeService::setup() const
{
    pinMode(GPIO_NUM_26, INPUT_PULLUP);
    Log.traceln("Attach interrupt");
    attachRotationInterrupt();
}

void StrokeService::calculateDragCoefficient()
{
    auto recoveryEndAngularVelocity = 2 * PI / cleanDeltaTimes[DELTA_TIME_ARRAY_LENGTH - 1];
    if (recoveryStartAngularVelocity > recoveryEndAngularVelocity && recoveryDuration < MAX_DRAG_FACTOR_RECOVERY_PERIOD * 1000)
    {
        auto rawNewDragCoefficient = -1 * FLYWHEEL_INERTIA * ((1 / recoveryStartAngularVelocity) - (1 / recoveryEndAngularVelocity)) / recoveryDuration;

        if (rawNewDragCoefficient < UPPER_DRAG_FACTOR_THRESHOLD &&
            rawNewDragCoefficient > LOWER_DRAG_FACTOR_THRESHOLD)
        {

            // auto newDragCoefficient = any_of(dragCoefficients.cbegin(),
            //                                  dragCoefficients.cend(),
            //                                  [](double item)
            //                                  { return item == 0; })
            //                               ? rawNewDragCoefficient
            //                               : accumulate(dragCoefficients.cbegin(), dragCoefficients.cend(), rawNewDragCoefficient) / (dragCoefficients.size() + 1);

            auto newDragCoefficient = rawNewDragCoefficient;

            char i = dragCoefficients.size() - 1;
            while (i > 0)
            {
                dragCoefficients[i] = dragCoefficients[i - 1];
                i--;
            }
            dragCoefficients[0] = newDragCoefficient;

            array<double, DRAG_COEFFICIENTS_ARRAY_LENGTH> sortedArray{};

            partial_sort_copy(dragCoefficients.cbegin(), dragCoefficients.cend(), sortedArray.begin(), sortedArray.end());
            dragCoefficient = sortedArray[sortedArray.size() / 2];
        }
    }
}

void StrokeService::calculateAvgStrokePower()
{
    avgStrokePower = dragCoefficient * pow((revCount - 1 - driveStartRevCount) * 2 * PI / ((lastDriveDuration + recoveryDuration) / 1e6), 3);
    // Log.infoln("test: %D", pow((revCount - 1 - driveStartRevCount) * 2 * PI / ((lastDriveDuration + recoveryDuration) / 1e6), 3));
    // Log.infoln("test: %d", revCount);
    // Log.infoln("test: %d", driveStartRevCount);
    // Log.infoln("test: %d", recoveryStartRevCount);
    // Log.infoln("test: %d", (revCount - 1 - driveStartRevCount));
    // Log.infoln("test: %D", (lastDriveDuration + recoveryDuration) / 1e6);
    // Log.infoln("test: %D", avgStrokePower);
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
        .driveDuration = lastDriveDuration,
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
    auto currentDeltaTime = now - previousRawRevTime;

    if (currentDeltaTime < ROTATION_DEBOUNCE_TIME_MIN * 1000)
        return;

    auto deltaTimeDiffPair = minmax<volatile unsigned long>(currentDeltaTime, previousDeltaTime);
    auto deltaTimeDiff = deltaTimeDiffPair.second - deltaTimeDiffPair.first;

    previousDeltaTime = currentDeltaTime;
    // We disregard rotation signals that are non sensible (the absolute difference of the current and the previous deltas exceeds the current delta)
    if (deltaTimeDiff > currentDeltaTime)
        return;

    // If we got this far, we must have a sensible delta for flywheel rotation time, updating the deltaTime array
    char i = cleanDeltaTimes.size() - 1;
    while (i > 0)
    {
        cleanDeltaTimes[i] = cleanDeltaTimes[i - 1];
        i--;
    }
    cleanDeltaTimes[0] = currentDeltaTime;

    previousRawRevTime = now;

    // If rotation delta exceeds the max debounce time and we are in Recovery Phase, the rower must have stopped. Setting cyclePhase to "Stopped"
    // TODO: decide whether the recovery duration should be checked instead
    if (cyclePhase == CyclePhase::Recovery && recoveryDuration > ROWING_STOPPED_THRESHOLD_PERIOD * 1000)
    {
        cyclePhase = CyclePhase::Stopped;

        recoveryDuration = 0;
        driveDuration = 0;
        avgStrokePower = 0;

        return;
    }

    if (cyclePhase == CyclePhase::Stopped)
    {
        // We are currently in the "Stopped" phase, as power was not applied for a long period of time or the device just started. Since rotation was detected we check if cleanDeltaTimes array is filled (i.e. whether we have sufficient data for determining the next phase) and whether power is being applied to the flywheel
        if (
            any_of(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cend(), [](unsigned long cleanDeltaTime)
                   { return cleanDeltaTime == 0; }) ||
            isFlywheelUnpowered())
            return;

        // Since we detected power, setting to "Drive" phase and increasing rotation count and registering rotation time
        cyclePhase = CyclePhase::Drive;
        driveStartTime = now - cleanDeltaTimes[0];
        driveStartRevCount = revCount; // no need to subtract 1 from revCount as it is not incremented yet

        lastRevTime = now;
        revCount++;

        return;
    }

    // We add a new rotation since we are not in the "Stopped" phase
    lastRevTime = now;
    revCount++;

    // we implement a finite state machine that goes between "Drive" and "Recovery" phases while paddeling on the machine. This allows a phase-change if sufficient time has passed and there is a plausible flank
    if (cyclePhase == CyclePhase::Drive)
    {
        // We are currently in the "Drive" phase, lets determine what the next phase is (if we come from "Stopped" phase )
        if (isFlywheelUnpowered())
        {
            // It seems that we lost power to the flywheel lets check if drive time was sufficint for detecting a stroke (i.e. drivePhaseDuration exceeds debounce time)
            if (driveDuration > STROKE_DEBOUNCE_TIME * 1000)
            {
                // Here we can conclude the "Drive" phase as there is no more drive detected to the flywheel (e.g. for calculating power etc.)
                strokeCount++;
                lastDriveDuration = driveDuration;
                lastStrokeTime = now;
            }

            cyclePhase = CyclePhase::Recovery;
            recoveryStartTime = now - cleanDeltaTimes[0];
            recoveryStartAngularVelocity = 2 * PI / cleanDeltaTimes[0];
            recoveryStartRevCount = revCount - 1;
            driveDuration = 0;
            return;
        }

        driveDuration = now - driveStartTime;
        return;
    }

    if (cyclePhase == CyclePhase::Recovery)
    {
        // We are currently in the "Recovery" phase, lets determine what the next phase is
        if (isFlywheelPowered())
        {
            // Here we can conclude the "Recovery" phase (and the current stroke cycle) as drive to the flywheel is detected (e.g. calculating drag factor)
            calculateDragCoefficient();
            calculateAvgStrokePower();

            cyclePhase = CyclePhase::Drive;
            driveStartTime = now - cleanDeltaTimes[0];
            driveStartRevCount = revCount - 1;
            recoveryDuration = 0;
            return;
        }

        recoveryDuration = now - recoveryStartTime;
        return;
    }
}

// c(2.8) * (distance / time)^3 = P

// driveLinearDistance = Math.pow((dragFactor(1) / rowerSettings.magicConstant(2.8)), 1.0 / 3.0) * ((totalNumberOfImpulses - flankDetector.noImpulsesToBeginFlank()) - drivePhaseStartAngularDisplacement) * (2.0 * Math.PI) / rowerSettings.numOfImpulsesPerRevolution(1)

// driveLinearDistance = Math.pow((dragFactor(1) / rowerSettings.magicConstant(2.8)), 1.0 / 3.0) * ((totalNumberOfImpulses - drivePhaseStartAngularDisplacement"impulses") *  (2.0 * Math.PI) / rowerSettings.numOfImpulsesPerRevolution(1)
// *angularVelocity = (2.0 * Math.PI) / deltaTime
// currentDragFactor = -1 * rowerSettings.flywheelInertia(0.0802) * ((1 / recoveryStartAngularVelocity) - (1 / recoveryEndAngularVelocity)) / recoveryPhaseLength