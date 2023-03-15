#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>

#include "Arduino.h"

#include "ArduinoLog.h"

#include "globals.h"
#include "stroke.service.h"

using std::accumulate;
using std::any_of;
using std::array;
using std::minmax;
using std::partial_sort_copy;

using RowingDataModels::RowingMetrics;

StrokeService::StrokeService()
{
    angularVelocityMatrix.reserve(Settings::impulseDataArrayLength);
    angularAccelerationMatrix.reserve(Settings::impulseDataArrayLength);
}

bool StrokeService::isFlywheelUnpowered() const
{
    return currentTorque < Settings::minimumDragTorque;
}

bool StrokeService::isFlywheelPowered() const
{
    return currentTorque > Settings::minimumPoweredTorque && deltaTimes.slope() < 0;
}

void StrokeService::calculateDragCoefficient()
{
    if (recoveryDuration > Settings::maxDragFactorRecoveryPeriod * 1000)
    {
        return;
    }

    if (recoveryDeltaTimes.goodnessOfFit() < Settings::goodnessOfFitThreshold)
    {
        return;
    }

    auto rawNewDragCoefficient = (recoveryDeltaTimes.slope() * Settings::flywheelInertia) / angularDisplacementPerImpulse;

    if (rawNewDragCoefficient > Settings::upperDragFactorThreshold ||
        rawNewDragCoefficient < Settings::lowerDragFactorThreshold)
    {
        return;
    }

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
    // Log.infoln("dragfactorraw: %.10f", dragCoefficient);
}

void StrokeService::calculateAvgStrokePower()
{
    avgStrokePower = dragCoefficient * pow((recoveryTotalAngularDisplacement + driveTotalAngularDisplacement) / ((driveDuration + recoveryDuration) / 1e6), 3);
}

void StrokeService::driveStart()
{
    cyclePhase = CyclePhase::Drive;
    driveStartTime = rowingTotalTime;
    driveStartAngularDisplacement = rowingTotalAngularDisplacement;
    driveHandleForces.clear();
    driveHandleForces.push_back(currentTorque / sprocketRadius);
}

void StrokeService::driveUpdate()
{
    driveHandleForces.push_back(currentTorque / sprocketRadius);
}

void StrokeService::driveEnd()
{
    // It seems that we lost power to the flywheel lets check if drive time was sufficient for detecting a stroke (i.e. drivePhaseDuration exceeds debounce time)

    driveDuration = rowingTotalTime - driveStartTime;
    driveTotalAngularDisplacement = rowingTotalAngularDisplacement - driveStartAngularDisplacement;
    if (driveDuration > Settings::strokeDebounceTime * 1000)
    {
        // Here we can conclude the "Drive" phase as there is no more drive detected to the flywheel (e.g. for calculating power etc.)
        strokeCount++;
        strokeTime = rowingTotalTime;
    }
    distance += pow((dragCoefficient * 1e6) / Settings::concept2MagicNumber, 1 / 3.0) * driveTotalAngularDisplacement;
    revTime = rowingTotalTime;

    // printf("forceCurve: [");
    // for (const auto &force : driveHandleForces)
    // {
    //     printf("%.10f,", force);
    // }
    // printf("]\n");
}

void StrokeService::recoveryStart()
{
    cyclePhase = CyclePhase::Recovery;
    recoveryStartTime = rowingTotalTime;
    recoveryStartAngularDisplacement = rowingTotalAngularDisplacement;
    recoveryDeltaTimes.reset();
    recoveryDeltaTimes.push(rowingTotalTime, deltaTimes.yAtSeriesBegin());
}

void StrokeService::recoveryUpdate()
{
    recoveryDeltaTimes.push(rowingTotalTime, deltaTimes.yAtSeriesBegin());
}

void StrokeService::recoveryEnd()
{
    recoveryDuration = rowingTotalTime - recoveryStartTime;
    recoveryTotalAngularDisplacement = rowingTotalAngularDisplacement - recoveryStartAngularDisplacement;
    calculateDragCoefficient();
    calculateAvgStrokePower();

    distance += pow((dragCoefficient * 1e6) / Settings::concept2MagicNumber, 1 / 3.0) * (distance == 0 ? rowingTotalAngularDisplacement : recoveryTotalAngularDisplacement);
    revTime = rowingTotalTime;
}

RowingDataModels::RowingMetrics StrokeService::getData()
{
    return RowingDataModels::RowingMetrics{
        .distance = distance,
        .lastRevTime = revTime,
        .lastStrokeTime = strokeTime,
        .strokeCount = strokeCount,
        .driveDuration = driveDuration,
        .recoveryDuration = recoveryDuration,
        .avgStrokePower = avgStrokePower,
        .dragCoefficient = dragCoefficient,
        .driveHandleForces = driveHandleForces};
}

void StrokeService::processData(RowingDataModels::FlywheelData data)
{
    deltaTimes.push(data.totalTime, data.deltaTime);
    angularDistances.push(data.totalTime / 1e6, data.totalAngularDisplacement);

    if (angularVelocityMatrix.size() >= Settings::impulseDataArrayLength)
    {
        angularVelocityMatrix.erase(angularVelocityMatrix.begin());
    }
    if (angularAccelerationMatrix.size() >= Settings::impulseDataArrayLength)
    {
        angularAccelerationMatrix.erase(angularAccelerationMatrix.begin());
    }

    angularVelocityMatrix.push_back(Series(Settings::impulseDataArrayLength));
    angularAccelerationMatrix.push_back(Series(Settings::impulseDataArrayLength));

    unsigned char i = 0;
    while (i < angularVelocityMatrix.size())
    {
        angularVelocityMatrix[i].push(angularDistances.firstDerivativeAtPosition(i));
        angularAccelerationMatrix[i].push(angularDistances.secondDerivativeAtPosition(i));
        i++;
    }

    currentAngularVelocity = angularVelocityMatrix[0].median();
    currentAngularAcceleration = angularAccelerationMatrix[0].median();

    currentTorque = Settings::flywheelInertia * currentAngularAcceleration + dragCoefficient * pow(currentAngularVelocity, 2);

    // Log.infoln("currentTorque: %f", currentTorque);
    // Log.infoln("slope: %f", deltaTimes.slope());
    // If rotation delta exceeds the max debounce time and we are in Recovery Phase, the rower must have stopped. Setting cyclePhase to "Stopped"
    if (cyclePhase == CyclePhase::Recovery && rowingTotalTime - recoveryStartTime > Settings::rowingStoppedThresholdPeriod)
    {
        recoveryEnd();
        cyclePhase = CyclePhase::Stopped;
        driveDuration = 0;
        recoveryDuration = 0;
        avgStrokePower = 0;

        return;
    }

    if (cyclePhase == CyclePhase::Stopped)
    {
        // We are currently in the "Stopped" phase, as power was not applied for a long period of time or the device just started. Since rotation was detected we check if cleanDeltaTimes array is filled (i.e. whether we have sufficient data for determining the next phase) and whether power is being applied to the flywheel
        if (
            deltaTimes.size() < Settings::impulseDataArrayLength || !isFlywheelPowered())
        {
            return;
        }

        rowingImpulseCount++;
        rowingTotalTime += deltaTimes.yAtSeriesBegin();
        rowingTotalAngularDisplacement += angularDisplacementPerImpulse;

        // Since we detected power, setting to "Drive" phase and increasing rotation count and registering rotation time
        driveStart();

        return;
    }

    rowingImpulseCount++;
    rowingTotalTime += deltaTimes.yAtSeriesBegin();
    rowingTotalAngularDisplacement += angularDisplacementPerImpulse;
    revTime = rowingTotalTime;
    // we implement a finite state machine that goes between "Drive" and "Recovery" phases while paddling on the machine. This allows a phase-change if sufficient time has passed and there is a plausible flank
    if (cyclePhase == CyclePhase::Drive)
    {
        // We are currently in the "Drive" phase, lets determine what the next phase is (if we come from "Stopped" phase )
        if (isFlywheelUnpowered())
        {
            driveEnd();
            recoveryStart();

            return;
        }

        driveUpdate();

        return;
    }

    if (cyclePhase == CyclePhase::Recovery)
    {
        // We are currently in the "Recovery" phase, lets determine what the next phase is
        if (isFlywheelPowered())
        {
            // Here we can conclude the "Recovery" phase (and the current stroke cycle) as drive to the flywheel is detected (e.g. calculating drag factor)
            recoveryEnd();
            driveStart();
            return;
        }

        recoveryUpdate();
        return;
    }
}