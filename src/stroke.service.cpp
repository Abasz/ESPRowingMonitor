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
using std::all_of;
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
        if (cleanDeltaTimes[i] >= cleanDeltaTimes[i - 1] || cleanDeltaTimes[i - 1] - cleanDeltaTimes[i] < Settings::MINIMUM_ACCELERATION_DELTA_TO_POWERED)
        {
            // Oldest interval (dataPoints[i]) is larger than the younger one (datapoint[i-1], as the distance is
            // fixed, we are accelerating
            numberOfAccelerations++;
        }
        i--;
    }

    // If not all of the data points are consistently increasing or they are actually decreasing report unpowered
    if (numberOfAccelerations >= Settings::FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD)
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
        if (cleanDeltaTimes[i] < cleanDeltaTimes[i - 1] && cleanDeltaTimes[i - 1] - cleanDeltaTimes[i] > Settings::MINIMUM_DECELERATION_DELTA_TO_UNPOWERED)
        {
            // Oldest interval (dataPoints[i]) is shorter than the younger one (datapoint[i-1], as the distance is fixed, we
            // discovered a deceleration
            numberOfDecelerations++;
        }
        i--;
    }
    // If not all of the data points are consistently decreasing or they are actually increasing report powered
    if (numberOfDecelerations >= Settings::FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD)
    {
        return false;
    }

    return true;
}

void StrokeService::setup() const
{
    pinMode(Settings::SENSOR_PIN_NUMBER, INPUT_PULLUP);
    Log.traceln("Attach interrupt");
    attachRotationInterrupt();
}

void StrokeService::calculateDragCoefficient()
{
    if (cleanDeltaTimes[Settings::DELTA_TIME_ARRAY_LENGTH - 1] < Settings::DRAG_FACTOR_ROTATION_DELTA_UPPER_THRESHOLD * 1000 || recoveryDuration > Settings::MAX_DRAG_FACTOR_RECOVERY_PERIOD * 1000)
        return;

    if (regressorService.goodnessOfFit() < Settings::GOODNESS_OF_FIT_THRESHOLD)
        return;

    auto rawNewDragCoefficient = (regressorService.slope() * Settings::FLYWHEEL_INERTIA) / ANGULAR_DISPLACEMENT_PER_IMPULSE;

    if (rawNewDragCoefficient > Settings::UPPER_DRAG_FACTOR_THRESHOLD ||
        rawNewDragCoefficient < Settings::LOWER_DRAG_FACTOR_THRESHOLD)
        return;

    if (Settings::DRAG_COEFFICIENTS_ARRAY_LENGTH > 1)
    {
        char i = Settings::DRAG_COEFFICIENTS_ARRAY_LENGTH - 1;
        while (i > 0)
        {
            dragCoefficients[i] = dragCoefficients[i - 1];
            i--;
        }
        dragCoefficients[0] = rawNewDragCoefficient;

        array<double, Settings::DRAG_COEFFICIENTS_ARRAY_LENGTH> sortedArray{};

        partial_sort_copy(dragCoefficients.cbegin(), dragCoefficients.cend(), sortedArray.begin(), sortedArray.end());
        rawNewDragCoefficient = sortedArray[sortedArray.size() / 2];
    }

    dragCoefficient = rawNewDragCoefficient;
}

void StrokeService::calculateAvgStrokePower()
{
    avgStrokePower = dragCoefficient * pow((impulseCount - 1 - driveStartImpulseCount) * ANGULAR_DISPLACEMENT_PER_IMPULSE / ((lastDriveDuration + recoveryDuration) / 1e6), 3);
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
    if (previousCleanRevTime != lastDataReadTime)
    {
        lastDataReadTime = previousCleanRevTime;

        return true;
    }

    return false;
}

void StrokeService::processRotation(unsigned long now)
{
    auto currentRawDeltaTime = now - previousRawRevTime;

    previousRawRevTime = now;

    if (currentRawDeltaTime < Settings::ROTATION_DEBOUNCE_TIME_MIN * 1000)
        return;

    auto currentCleanDeltaTime = now - previousCleanRevTime;

    auto deltaTimeDiffPair = minmax<volatile unsigned long>(currentCleanDeltaTime, previousDeltaTime);
    auto deltaTimeDiff = deltaTimeDiffPair.second - deltaTimeDiffPair.first;

    previousDeltaTime = currentCleanDeltaTime;
    // We disregard rotation signals that are non sensible (the absolute difference of the current and the previous deltas exceeds the current delta)
    // TODO: check if adding a constraint to being in the Recovery Phase would help with the data skipping in the drive phase where sudden high power if applied to the flywheel causing quick increase in the revtime. This needs to be tested with a low ROTATION_DEBOUNCE_TIME_MIN
    if (deltaTimeDiff > currentCleanDeltaTime && cyclePhase != CyclePhase::Drive)
        return;

    // If we got this far, we must have a sensible delta for flywheel rotation time, updating the deltaTime array
    byte i = cleanDeltaTimes.size() - 1;
    while (i > 0)
    {
        cleanDeltaTimes[i] = cleanDeltaTimes[i - 1];
        i--;
    }
    cleanDeltaTimes[0] = currentCleanDeltaTime;

    previousCleanRevTime = now;

    // If rotation delta exceeds the max debounce time and we are in Recovery Phase, the rower must have stopped. Setting cyclePhase to "Stopped"
    if (cyclePhase == CyclePhase::Recovery && recoveryDuration > Settings::ROWING_STOPPED_THRESHOLD_PERIOD * 1000)
    {
        cyclePhase = CyclePhase::Stopped;

        recoveryDuration = 0;
        driveDuration = 0;
        avgStrokePower = 0;
        impulseCount = 0;

        return;
    }

    if (cyclePhase == CyclePhase::Stopped)
    {
        // We are currently in the "Stopped" phase, as power was not applied for a long period of time or the device just started. Since rotation was detected we check if cleanDeltaTimes array is filled (i.e. whether we have sufficient data for determining the next phase) and whether power is being applied to the flywheel
        if (
            all_of(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cend(), [](unsigned long cleanDeltaTime)
                   { return cleanDeltaTime != 0; }) &&
            isFlywheelPowered())
        {
            // Since we detected power, setting to "Drive" phase and increasing rotation count and registering rotation time
            cyclePhase = CyclePhase::Drive;
            driveStartTime = now - accumulate(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cend() - Settings::FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD - 1, 0);
            driveStartImpulseCount = 0;

            impulseCount++;
            if (impulseCount % Settings::IMPULSES_PER_REVOLUTION == 0)
            {
                revCount++;
                lastRevTime = now;
            }

            return;
        }
    }

    impulseCount++;
    if (impulseCount % Settings::IMPULSES_PER_REVOLUTION == 0)
    {
        revCount++;
        lastRevTime = now;
    }

    // we implement a finite state machine that goes between "Drive" and "Recovery" phases while paddeling on the machine. This allows a phase-change if sufficient time has passed and there is a plausible flank
    if (cyclePhase == CyclePhase::Drive)
    {
        // We are currently in the "Drive" phase, lets determine what the next phase is (if we come from "Stopped" phase )
        if (isFlywheelUnpowered())
        {
            // It seems that we lost power to the flywheel lets check if drive time was sufficint for detecting a stroke (i.e. drivePhaseDuration exceeds debounce time)
            if (driveDuration > Settings::STROKE_DEBOUNCE_TIME * 1000)
            {
                // Here we can conclude the "Drive" phase as there is no more drive detected to the flywheel (e.g. for calculating power etc.)
                strokeCount++;
                lastDriveDuration = driveDuration;
                lastStrokeTime = now;
            }

            cyclePhase = CyclePhase::Recovery;
            dragTimer = cleanDeltaTimes[FLANK_START_INDEX];
            regressorService.addToDataset(dragTimer, cleanDeltaTimes[FLANK_START_INDEX]);
            recoveryStartTime = now - accumulate(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cend() - Settings::FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD, 0);
            driveDuration = 0;

            return;
        }

        driveDuration = now - driveStartTime - accumulate(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cend() - Settings::FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD - 1, 0);

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
            driveStartTime = now - accumulate(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cend() - Settings::FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD, 0);
            driveStartImpulseCount = impulseCount - 1;
            recoveryDuration = 0;
            dragTimer = 0;
            regressorService.resetData();

            return;
        }

        recoveryDuration = now - recoveryStartTime - accumulate(cleanDeltaTimes.cbegin(), cleanDeltaTimes.cend() - Settings::FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD - 1, 0);
        dragTimer += cleanDeltaTimes[FLANK_START_INDEX];
        regressorService.addToDataset(dragTimer, cleanDeltaTimes[FLANK_START_INDEX]);

        return;
    }
}