#pragma once

#include "../utils/configuration.h"
#include "../utils/series/ols-linear-series.h"
#include "../utils/series/ts-linear-series.h"
#include "../utils/series/ts-quadratic-series.h"
#include "../utils/series/weighted-average-series.h"
#include "./stroke.model.h"
#include "./stroke.service.interface.h"

class StrokeService final : public IStrokeService
{
    // rower state
    CyclePhase cyclePhase = CyclePhase::Stopped;
    unsigned long long rowingTotalTime = 0ULL;
    unsigned long long rowingImpulseCount = 0UL;
    Configurations::precision rowingTotalAngularDisplacement = 0;

    // Drive related
    unsigned long long driveStartTime = 0ULL;
    unsigned int driveDuration = 0;
    Configurations::precision driveStartAngularDisplacement = 0;
    Configurations::precision driveTotalAngularDisplacement = 0;

    // Recovery related
    unsigned long long recoveryStartTime = 0;
    unsigned int recoveryDuration = 0;
    Configurations::precision recoveryStartAngularDisplacement = 0;
    Configurations::precision recoveryTotalAngularDisplacement = 0;
    Configurations::precision recoveryStartDistance = 0;

    // metrics
    Configurations::precision distancePerAngularDisplacement = 0;
    Configurations::precision distance = 0;
    unsigned short strokeCount = 0;
    unsigned long long strokeTime = 0ULL;
    unsigned long long revTime = 0ULL;
    Configurations::precision avgStrokePower = 0;

    Configurations::precision dragCoefficient = 0;

    WeightedAverageSeries dragCoefficients = WeightedAverageSeries(Configurations::dragCoefficientsArrayLength);

    Configurations::precision totalAngularDisplacement = 0;

    // advance metrics
    Configurations::precision currentAngularVelocity = 0;
    Configurations::precision currentAngularAcceleration = 0;
    Configurations::precision currentTorque = 0;
    vector<float> driveHandleForces;

    vector<WeightedAverageSeries> angularVelocityMatrix;
    vector<WeightedAverageSeries> angularAccelerationMatrix;

    TSLinearSeries deltaTimes = TSLinearSeries(Configurations::impulseDataArrayLength);
    OLSLinearSeries deltaTimesSlopes = OLSLinearSeries(Configurations::impulseDataArrayLength);
    OLSLinearSeries recoveryDeltaTimes = OLSLinearSeries(0, Configurations::maxDragFactorRecoveryPeriod / Configurations::rotationDebounceTimeMin / 2);
    TSQuadraticSeries angularDistances = TSQuadraticSeries(Configurations::impulseDataArrayLength);

    bool isFlywheelUnpowered();
    bool isFlywheelPowered();
    void calculateDragCoefficient();
    void calculateAvgStrokePower();

    void driveStart();
    void driveUpdate();
    void driveEnd();
    void recoveryStart();
    void recoveryUpdate();
    void recoveryEnd();

    void logSlopeMarginDetection() const;
    void logNewStrokeData() const;

public:
    StrokeService();

    RowingDataModels::RowingMetrics getData() override;
    void processData(RowingDataModels::FlywheelData data) override;
};