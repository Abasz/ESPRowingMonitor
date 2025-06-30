#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>

#include "Arduino.h"
#include "ArduinoLog.h"

#include "globals.h"

#include "./stroke.service.h"

using RowingDataModels::RowingMetrics;

StrokeService::StrokeService()
{
    angularVelocityMatrix.reserve(RowerProfile::Defaults::impulseDataArrayLength);
    angularAccelerationMatrix.reserve(RowerProfile::Defaults::impulseDataArrayLength);
    driveHandleForces.reserve(RowerProfile::Defaults::driveHandleForcesMaxCapacity);

    deltaTimes.push(0, 0);
    angularDistances.push(0, 0);
}

#if ENABLE_RUNTIME_SETTINGS
void StrokeService::setup(const RowerProfile::MachineSettings newMachineSettings)
{
    machineSettings = newMachineSettings;
    angularDisplacementPerImpulse = (2 * PI) / machineSettings.impulsesPerRevolution;
}
#endif

bool StrokeService::isFlywheelUnpowered()
{
    if constexpr (RowerProfile::Defaults::strokeDetectionType != StrokeDetectionType::Slope)
    {
        if (deltaTimesSlopes.size() >= RowerProfile::Defaults::impulseDataArrayLength && ((currentTorque < RowerProfile::Defaults::minimumDragTorque && deltaTimes.coefficientA() > 0) || std::abs(deltaTimesSlopes.slope()) < RowerProfile::Defaults::minimumRecoverySlopeMargin))
        {
            if constexpr (Configurations::logCalibration)
            {
                logSlopeMarginDetection();
            }

            return true;
        }
    }

    if constexpr (RowerProfile::Defaults::strokeDetectionType != StrokeDetectionType::Torque)
    {
        if (deltaTimes.coefficientA() > RowerProfile::Defaults::minimumRecoverySlope)
        {
            return true;
        }
    }

    return false;
}

bool StrokeService::isFlywheelPowered()
{
    return currentTorque > RowerProfile::Defaults::minimumPoweredTorque && deltaTimes.coefficientA() < 0;
}

void StrokeService::calculateDragCoefficient()
{
    if (recoveryDuration > RowerProfile::Defaults::maxDragFactorRecoveryPeriod || recoveryDeltaTimes.size() < RowerProfile::Defaults::impulseDataArrayLength)
    {
        return;
    }

    const auto goodnessOfFit = recoveryDeltaTimes.goodnessOfFit();

    if (goodnessOfFit < RowerProfile::Defaults::goodnessOfFitThreshold)
    {
        return;
    }

    const auto rawNewDragCoefficient = (recoveryDeltaTimes.slope() * machineSettings.flywheelInertia) / angularDisplacementPerImpulse;

    if (rawNewDragCoefficient > RowerProfile::Defaults::upperDragFactorThreshold ||
        rawNewDragCoefficient < RowerProfile::Defaults::lowerDragFactorThreshold)
    {
        return;
    }

    if constexpr (RowerProfile::Defaults::dragCoefficientsArrayLength < 2)
    {
        dragCoefficient = rawNewDragCoefficient;
        lastValidDragCoefficient = dragCoefficient;

        return;
    }

    dragCoefficients.push(rawNewDragCoefficient, goodnessOfFit);

    dragCoefficient = dragCoefficients.average();
    lastValidDragCoefficient = dragCoefficient;
}

void StrokeService::calculateAvgStrokePower()
{
    avgStrokePower = lastValidDragCoefficient * pow((recoveryTotalAngularDisplacement + driveTotalAngularDisplacement) / ((driveDuration + recoveryDuration) / 1e6), 3);
}

void StrokeService::driveStart()
{
    cyclePhase = CyclePhase::Drive;
    driveStartTime = rowingTotalTime;
    driveStartAngularDisplacement = rowingTotalAngularDisplacement;

    driveHandleForces.clear();
    driveHandleForces.push_back(static_cast<float>(currentTorque) / machineSettings.sprocketRadius);

    if constexpr (RowerProfile::Defaults::strokeDetectionType != StrokeDetectionType::Slope)
    {
        deltaTimesSlopes.reset();
        deltaTimesSlopes.push(static_cast<Configurations::precision>(rowingTotalTime), deltaTimes.coefficientA());
    }
}

void StrokeService::driveUpdate()
{
    if (driveHandleForces.size() >= RowerProfile::Defaults::driveHandleForcesMaxCapacity)
    {
        driveEnd();
        if constexpr (RowerProfile::Defaults::strokeDetectionType != StrokeDetectionType::Slope)
        {
            dragCoefficient = 0;
            dragCoefficients.reset();
        }
        recoveryStart();

        return;
    }

    driveHandleForces.push_back(static_cast<float>(currentTorque) / machineSettings.sprocketRadius);
    if constexpr (RowerProfile::Defaults::strokeDetectionType != StrokeDetectionType::Slope)
    {
        deltaTimesSlopes.push(static_cast<Configurations::precision>(rowingTotalTime), deltaTimes.coefficientA());
    }
}

void StrokeService::driveEnd()
{
    // It seems that we lost power to the flywheel lets check if drive time was sufficient for detecting a stroke (i.e. drivePhaseDuration exceeds debounce time). So we can conclude the "Drive" phase as there is no more drive detected to the flywheel (e.g. for calculating power etc.)

    driveDuration = rowingTotalTime - driveStartTime;
    driveTotalAngularDisplacement = rowingTotalAngularDisplacement - driveStartAngularDisplacement;
    strokeCount++;
    strokeTime = rowingTotalTime;

    if constexpr (Configurations::logCalibration)
    {
        logNewStrokeData();
    }
}

void StrokeService::recoveryStart()
{
    cyclePhase = CyclePhase::Recovery;
    recoveryStartTime = rowingTotalTime;
    recoveryStartAngularDisplacement = rowingTotalAngularDisplacement;
    recoveryStartDistance = distance;
    recoveryDeltaTimes.push(static_cast<Configurations::precision>(rowingTotalTime), deltaTimes.yAtSeriesBegin());
}

void StrokeService::recoveryUpdate()
{
    if (rowingTotalTime - recoveryStartTime < RowerProfile::Defaults::maxDragFactorRecoveryPeriod)
    {
        recoveryDeltaTimes.push(static_cast<Configurations::precision>(rowingTotalTime), deltaTimes.yAtSeriesBegin());
    }
}

void StrokeService::recoveryEnd()
{
    recoveryDuration = rowingTotalTime - recoveryStartTime;
    recoveryTotalAngularDisplacement = rowingTotalAngularDisplacement - recoveryStartAngularDisplacement;
    calculateDragCoefficient();

    recoveryDeltaTimes.reset();
    calculateAvgStrokePower();

    distancePerAngularDisplacement = pow((lastValidDragCoefficient * 1e6) / machineSettings.concept2MagicNumber, 1 / 3.0);
    distance = recoveryStartDistance + distancePerAngularDisplacement * (distance == 0 ? rowingTotalAngularDisplacement : recoveryTotalAngularDisplacement);
    if (distance > 0)
    {
        revTime = rowingTotalTime;
    }
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
        .dragCoefficient = lastValidDragCoefficient,
        .driveHandleForces = driveHandleForces,
    };
}

void StrokeService::processData(const RowingDataModels::FlywheelData data)
{
    deltaTimes.push(static_cast<Configurations::precision>(data.totalTime), static_cast<Configurations::precision>(data.deltaTime));
    angularDistances.push(static_cast<Configurations::precision>(data.totalTime) / 1e6, data.totalAngularDisplacement);

    if (angularVelocityMatrix.size() >= RowerProfile::Defaults::impulseDataArrayLength)
    {
        angularVelocityMatrix.erase(begin(angularVelocityMatrix));
    }
    if (angularAccelerationMatrix.size() >= RowerProfile::Defaults::impulseDataArrayLength)
    {
        angularAccelerationMatrix.erase(begin(angularAccelerationMatrix));
    }

    angularVelocityMatrix.push_back(WeightedAverageSeries(RowerProfile::Defaults::impulseDataArrayLength, Configurations::defaultAllocationCapacity));
    angularAccelerationMatrix.push_back(WeightedAverageSeries(RowerProfile::Defaults::impulseDataArrayLength, Configurations::defaultAllocationCapacity));

    unsigned char i = 0;
    const auto angularGoodnessOfFit = angularDistances.goodnessOfFit();
    while (i < angularVelocityMatrix.size())
    {
        angularVelocityMatrix[i].push(angularDistances.firstDerivativeAtPosition(i), angularGoodnessOfFit);
        angularAccelerationMatrix[i].push(angularDistances.secondDerivativeAtPosition(i), angularGoodnessOfFit);
        ++i;
    }

    currentAngularVelocity = angularVelocityMatrix[0].average();
    currentAngularAcceleration = angularAccelerationMatrix[0].average();

    currentTorque = machineSettings.flywheelInertia * currentAngularAcceleration + dragCoefficient * pow(currentAngularVelocity, 2);

    // If rotation delta exceeds the max debounce time and we are in Recovery Phase, the rower must have stopped. Setting cyclePhase to "Stopped"
    if (cyclePhase == CyclePhase::Recovery && rowingTotalTime - recoveryStartTime > RowerProfile::Defaults::rowingStoppedThresholdPeriod)
    {
        driveHandleForces.clear();
        recoveryEnd();
        cyclePhase = CyclePhase::Stopped;
        driveDuration = 0;
        avgStrokePower = 0;

        return;
    }

    if (cyclePhase == CyclePhase::Stopped)
    {
        // We are currently in the "Stopped" phase, as power was not applied for a long period of time or the device just started. Since rotation was detected we check if cleanDeltaTimes array is filled (i.e. whether we have sufficient data for determining the next phase) and whether power is being applied to the flywheel
        if (
            deltaTimes.size() < RowerProfile::Defaults::impulseDataArrayLength || !isFlywheelPowered())
        {
            return;
        }

        rowingImpulseCount++;
        rowingTotalTime += static_cast<long long>(deltaTimes.yAtSeriesBegin());
        revTime = rowingTotalTime;
        rowingTotalAngularDisplacement += angularDisplacementPerImpulse;

        // Since we detected power, setting to "Drive" phase and increasing rotation count and registering rotation time
        driveStart();

        return;
    }

    rowingImpulseCount++;
    rowingTotalTime += static_cast<long long>(deltaTimes.yAtSeriesBegin());
    rowingTotalAngularDisplacement += angularDisplacementPerImpulse;

    distance += distancePerAngularDisplacement * (distance == 0 ? rowingTotalAngularDisplacement : angularDisplacementPerImpulse);
    if (distance > 0)
    {
        revTime = rowingTotalTime;
    }
    // we implement a finite state machine that goes between "Drive" and "Recovery" phases while paddling on the machine. This allows a phase-change if sufficient time has passed and there is a plausible flank
    if (cyclePhase == CyclePhase::Drive)
    {
        // We are currently in the "Drive" phase, lets determine what the next phase is (if we come from "Stopped" phase )
        if (rowingTotalTime - driveStartTime > RowerProfile::Defaults::minimumDriveTime && isFlywheelUnpowered())
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
        if (rowingTotalTime - recoveryStartTime > RowerProfile::Defaults::minimumRecoveryTime && isFlywheelPowered())
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

void StrokeService::logSlopeMarginDetection() const
{
    if (deltaTimesSlopes.size() >= RowerProfile::Defaults::impulseDataArrayLength && currentTorque > RowerProfile::Defaults::minimumDragTorque && std::abs(deltaTimesSlopes.slope()) < RowerProfile::Defaults::minimumRecoverySlopeMargin)
    {
        Log.infoln("slope margin detect");
    }
}

void StrokeService::logNewStrokeData() const
{
    Log.infoln("deltaTime: %d", strokeCount);

    string response;

    response.append("[");

    for (const auto &handleForce : driveHandleForces)
    {
        response.append(std::to_string(handleForce) + ",");
    }

    if (!driveHandleForces.empty())
    {
        response.pop_back();
    }
    response.append("]}");

    Log.infoln("handleForces: %s", response.c_str());
}
