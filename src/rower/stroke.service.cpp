#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>

#include "Arduino.h"
#include "ArduinoLog.h"

#include "./stroke.service.h"

#include "../utils/configuration.h"
#include "../utils/enums.h"
#include "../utils/macros.h"
#include "../utils/series/cyclic-error-filter.h"
#include "../utils/series/ols-linear-series.h"
#include "../utils/series/series.h"
#include "../utils/series/ts-linear-series.h"
#include "../utils/series/ts-quadratic-series.h"
#include "../utils/series/weighted-average-series.h"
#include "../utils/settings.model.h"
#include "./stroke.model.h"

using namespace std::string_literals;

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
void StrokeService::setup(const RowerProfile::MachineSettings newMachineSettings, const RowerProfile::SensorSignalSettings newSensorSignalSettings, const RowerProfile::DragFactorSettings newDragFactorSettings, RowerProfile::StrokePhaseDetectionSettings newStrokeDetectionSettings)
{
    machineSettings = newMachineSettings;
    dragFactorSettings = newDragFactorSettings;
    strokePhaseDetectionSettings = newStrokeDetectionSettings;

    rowingStoppedThresholdPeriod = newSensorSignalSettings.rowingStoppedThresholdPeriod;
    angularDisplacementPerImpulse = (2 * PI) / machineSettings.impulsesPerRevolution;
    absoluteMinimumRecoveryDeltaTimesSize = std::max(newStrokeDetectionSettings.impulseDataArrayLength / 2 + 1, 3);

    dragCoefficients = WeightedAverageSeries(dragFactorSettings.dragCoefficientsArrayLength, Configurations::defaultAllocationCapacity);

    deltaTimes = TSLinearSeries(newStrokeDetectionSettings.impulseDataArrayLength, Configurations::defaultAllocationCapacity);
    deltaTimesSlopes = OLSLinearSeries(newStrokeDetectionSettings.impulseDataArrayLength, Configurations::defaultAllocationCapacity);
    recoveryDeltaTimes = OLSLinearSeries(0, Configurations::defaultAllocationCapacity, newDragFactorSettings.maxDragFactorRecoveryPeriod / newSensorSignalSettings.rotationDebounceTimeMin / 2);
    angularDistances = TSQuadraticSeries(newStrokeDetectionSettings.impulseDataArrayLength, Configurations::defaultAllocationCapacity);
    cyclicFilter = CyclicErrorFilter(
        newMachineSettings.impulsesPerRevolution,
        newStrokeDetectionSettings.impulseDataArrayLength,
        newSensorSignalSettings.cyclicErrorAggressiveness,
        Configurations::defaultAllocationCapacity,
        newDragFactorSettings.maxDragFactorRecoveryPeriod / newSensorSignalSettings.rotationDebounceTimeMin / 2);

    angularVelocityMatrix.clear();
    angularVelocityMatrix.shrink_to_fit();
    angularAccelerationMatrix.clear();
    angularAccelerationMatrix.shrink_to_fit();
    driveHandleForces.clear();
    driveHandleForces.shrink_to_fit();
    angularVelocityMatrix.reserve(strokePhaseDetectionSettings.impulseDataArrayLength);
    angularAccelerationMatrix.reserve(strokePhaseDetectionSettings.impulseDataArrayLength);
    driveHandleForces.reserve(strokePhaseDetectionSettings.driveHandleForcesMaxCapacity);

    deltaTimes.push(0, 0);
    angularDistances.push(0, 0);
}
#endif

void StrokeService::processFilterBuffer()
{
    if (cyclePhase == CyclePhase::Recovery)
    {
        return;
    }

    cyclicFilter.processNextRawDatapoint();
}

bool StrokeService::isFlywheelUnpowered()
{
    if (strokePhaseDetectionSettings.strokeDetectionType != StrokeDetectionType::Slope)
    {
        if (deltaTimesSlopes.size() >= strokePhaseDetectionSettings.impulseDataArrayLength && currentTorque < strokePhaseDetectionSettings.minimumDragTorque && deltaTimes.coefficientA() > 0)
        {
            return true;
        }
    }

    if (strokePhaseDetectionSettings.strokeDetectionType != StrokeDetectionType::Torque)
    {
        if (deltaTimes.coefficientA() > strokePhaseDetectionSettings.minimumRecoverySlope)
        {
            return true;
        }
    }

    return false;
}

bool StrokeService::isFlywheelPowered()
{
    return currentTorque > strokePhaseDetectionSettings.minimumPoweredTorque && deltaTimes.coefficientA() < 0;
}

Configurations::precision StrokeService::calculateRecoveryGoodnessOfFit() const
{
    if (recoveryDuration > dragFactorSettings.maxDragFactorRecoveryPeriod)
    {
        return 0;
    }

    auto minRequiredRecoveryDeltaTimes = (dragCoefficient == 0)
                                             ? absoluteMinimumRecoveryDeltaTimesSize
                                             : strokePhaseDetectionSettings.impulseDataArrayLength;
    if (recoveryDeltaTimes.size() < minRequiredRecoveryDeltaTimes)
    {
        return 0;
    }

    return recoveryDeltaTimes.goodnessOfFit();
}

void StrokeService::calculateDragCoefficient(Configurations::precision goodnessOfFit)
{
    const auto rawNewDragCoefficient = (recoveryDeltaTimes.slope() * machineSettings.flywheelInertia) / angularDisplacementPerImpulse;

    if (rawNewDragCoefficient > dragFactorSettings.upperDragFactorThreshold ||
        rawNewDragCoefficient < dragFactorSettings.lowerDragFactorThreshold)
    {
        return;
    }

    if (dragFactorSettings.dragCoefficientsArrayLength < 2)
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
    avgStrokePower = lastValidDragCoefficient * std::pow((recoveryTotalAngularDisplacement + driveTotalAngularDisplacement) / ((driveDuration + recoveryDuration) / 1e6), 3);
}

void StrokeService::driveStart()
{
    cyclePhase = CyclePhase::Drive;
    driveStartTime = rowingTotalTime;
    driveStartAngularDisplacement = rowingTotalAngularDisplacement;

    driveHandleForces.clear();

    if (torqueBeforeFlank > (Configurations::precision)0)
    {
        driveHandleForces.push_back(static_cast<float>(torqueBeforeFlank) / machineSettings.sprocketRadius);
    }

    if (strokePhaseDetectionSettings.strokeDetectionType != StrokeDetectionType::Slope)
    {
        deltaTimesSlopes.reset();
        deltaTimesSlopes.push(static_cast<Configurations::precision>(rowingTotalTime), deltaTimes.coefficientA());
    }
}

void StrokeService::driveUpdate()
{
    if (driveHandleForces.size() >= strokePhaseDetectionSettings.driveHandleForcesMaxCapacity)
    {
        driveEnd();
        if (strokePhaseDetectionSettings.strokeDetectionType != StrokeDetectionType::Slope)
        {
            dragCoefficient = 0;
            dragCoefficients.reset();
        }
        recoveryStart();

        return;
    }

    driveHandleForces.push_back(static_cast<float>(torqueBeforeFlank) / machineSettings.sprocketRadius);
    if (strokePhaseDetectionSettings.strokeDetectionType != StrokeDetectionType::Slope)
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
    recoveryDeltaTimes.push(deltaTimes.xAtSeriesBegin(), deltaTimes.yAtSeriesBegin());

    cyclicFilter.restart();
}

void StrokeService::recoveryUpdate()
{
    if (rowingTotalTime - recoveryStartTime >= dragFactorSettings.maxDragFactorRecoveryPeriod)
    {
        return;
    }

    recoveryDeltaTimes.push(deltaTimes.xAtSeriesBegin(), deltaTimes.yAtSeriesBegin());

    const auto adjustedPosition = rawImpulseCount - strokePhaseDetectionSettings.impulseDataArrayLength + 1;
    cyclicFilter.recordRawDatapoint(
        adjustedPosition,
        deltaTimes.xAtSeriesBegin(),
        cyclicFilter.rawSeries().front());
}

void StrokeService::recoveryEnd()
{
    recoveryDuration = rowingTotalTime - recoveryStartTime;
    recoveryTotalAngularDisplacement = rowingTotalAngularDisplacement - recoveryStartAngularDisplacement;

    const auto goodnessOfFit = calculateRecoveryGoodnessOfFit();
    if (goodnessOfFit >= dragFactorSettings.goodnessOfFitThreshold)
    {
        calculateDragCoefficient(goodnessOfFit);
    }
    else if (!cyclicFilter.isPotentiallyMisaligned() || goodnessOfFit == 0.0F)
    {
        cyclicFilter.restart();
    }

    cyclicFilter.updateRegressionCoefficients(
        recoveryDeltaTimes.slope(),
        recoveryDeltaTimes.intercept(),
        goodnessOfFit);

    recoveryDeltaTimes.reset();
    calculateAvgStrokePower();

    distancePerAngularDisplacement = std::pow((lastValidDragCoefficient * 1e6) / machineSettings.concept2MagicNumber, 1 / 3.0);
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

void StrokeService::updateStrokeState(const RowingDataModels::FlywheelData data)
{
    rawImpulseCount = data.rawImpulseCount;

    cyclicFilter.applyFilter(data.rawImpulseCount, static_cast<Configurations::precision>(data.deltaTime));

    if constexpr (Configurations::logCalibration)
    {
        Log.infoln("%.2f,%.2f", cyclicFilter.rawSeries().back(), cyclicFilter.cleanSeries().back());
    }

    auto deltaTime = cyclicFilter.cleanSeries().back();
    const auto totalCleanTime = deltaTimes.xAtSeriesEnd() + deltaTime;

    deltaTimes.push(static_cast<Configurations::precision>(totalCleanTime), deltaTime);
    angularDistances.push(static_cast<Configurations::precision>(totalCleanTime) / 1e6, data.totalAngularDisplacement);

    if (angularVelocityMatrix.size() >= strokePhaseDetectionSettings.impulseDataArrayLength)
    {
        angularVelocityMatrix.erase(begin(angularVelocityMatrix));
    }
    if (angularAccelerationMatrix.size() >= strokePhaseDetectionSettings.impulseDataArrayLength)
    {
        angularAccelerationMatrix.erase(begin(angularAccelerationMatrix));
    }

    angularVelocityMatrix.emplace_back(strokePhaseDetectionSettings.impulseDataArrayLength, Configurations::defaultAllocationCapacity);
    angularAccelerationMatrix.emplace_back(strokePhaseDetectionSettings.impulseDataArrayLength, Configurations::defaultAllocationCapacity);

    const auto angularGoodnessOfFit = angularDistances.goodnessOfFit();
    const auto angularVelocitySize = angularVelocityMatrix.size();
    unsigned char i = 0;
    while (i < angularVelocitySize)
    {
        angularVelocityMatrix[i].push(angularDistances.firstDerivativeAtPosition(i), angularGoodnessOfFit);
        angularAccelerationMatrix[i].push(angularDistances.secondDerivativeAtPosition(i), angularGoodnessOfFit);
        ++i;
    }

    currentAngularVelocity = angularVelocityMatrix[0].average();
    currentAngularAcceleration = angularAccelerationMatrix[0].average();
}

void StrokeService::updatePhysics()
{
    torqueBeforeFlank = currentTorque;
    currentTorque = machineSettings.flywheelInertia * currentAngularAcceleration + dragCoefficient * std::pow(currentAngularVelocity, 2);
}

void StrokeService::applyImpulse()
{
    rowingImpulseCount++;
    rowingTotalTime += static_cast<unsigned long long>(cyclicFilter.cleanSeries().back());
    rowingTotalAngularDisplacement += angularDisplacementPerImpulse;

    distance += distancePerAngularDisplacement * (distance == 0 ? rowingTotalAngularDisplacement : angularDisplacementPerImpulse);
    if (distance > 0)
    {
        revTime = rowingTotalTime;
    }
}

void StrokeService::processStopped()
{
    // Wait until the delta times array is filled and power is detected before
    // starting a drive — ensures sufficient data for phase detection
    if (deltaTimes.size() < strokePhaseDetectionSettings.impulseDataArrayLength || !isFlywheelPowered())
    {
        return;
    }

    rowingImpulseCount++;
    rowingTotalTime += static_cast<unsigned long long>(cyclicFilter.cleanSeries().back());
    revTime = rowingTotalTime;
    rowingTotalAngularDisplacement += angularDisplacementPerImpulse;

    driveStart();
}

void StrokeService::processDrive()
{
    applyImpulse();

    // Conclude Drive if sufficient drive time has elapsed and the flywheel
    // is no longer powered
    if (rowingTotalTime - driveStartTime > strokePhaseDetectionSettings.minimumDriveTime && isFlywheelUnpowered())
    {
        driveEnd();
        recoveryStart();
        return;
    }

    driveUpdate();
}

void StrokeService::processRecovery()
{
    // If the rower has been idle long enough during Recovery, conclude the
    // phase and return to Stopped
    if (rowingTotalTime - recoveryStartTime > rowingStoppedThresholdPeriod)
    {
        driveHandleForces.clear();
        recoveryEnd();
        cyclePhase = CyclePhase::Stopped;
        driveDuration = 0;
        avgStrokePower = 0;
        return;
    }

    applyImpulse();

    // Conclude Recovery if sufficient recovery time has elapsed and drive is
    // detected on the flywheel — begins the next stroke cycle
    if (rowingTotalTime - recoveryStartTime > strokePhaseDetectionSettings.minimumRecoveryTime && isFlywheelPowered())
    {
        recoveryEnd();
        driveStart();
        return;
    }

    recoveryUpdate();
}

void StrokeService::processData(const RowingDataModels::FlywheelData data)
{
    updateStrokeState(data);
    updatePhysics();

    switch (cyclePhase)
    {
    case CyclePhase::Stopped:
        processStopped();
        return;
    case CyclePhase::Drive:
        processDrive();
        return;
    case CyclePhase::Recovery:
        processRecovery();
        return;
    }
    std::unreachable();
}

void StrokeService::logNewStrokeData() const
{
    Log.infoln("deltaTime: %d", strokeCount);

    if (driveHandleForces.empty())
    {
        Log.infoln("handleForces: []");

        return;
    }

    std::string formatted;
    for (size_t i = 0; i < driveHandleForces.size(); ++i)
    {
        if (i != 0)
        {
            formatted += ',';
        }
        formatted += std::to_string(driveHandleForces[i]);
    }

    string response = "[" + formatted + "]";

    Log.infoln("handleForces: %s", response.c_str());
}
