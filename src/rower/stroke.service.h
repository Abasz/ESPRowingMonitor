#pragma once

#include <array>

#include "../utils/configuration.h"
#include "../utils/ols-linear-series.h"
#include "../utils/series.h"
#include "../utils/ts-quadratic-series.h"
#include "stroke.model.h"

class StrokeService
{
    static Configurations::precision constexpr angularDisplacementPerImpulse = (2 * PI) / Configurations::impulsesPerRevolution;
    static Configurations::precision constexpr sprocketRadius = Configurations::sprocketRadius / 100;

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

    std::array<Configurations::precision, Configurations::dragCoefficientsArrayLength> dragCoefficients{};

    Configurations::precision totalAngularDisplacement = 0;

    // advance metrics
    Configurations::precision currentAngularVelocity = 0;
    Configurations::precision currentAngularAcceleration = 0;
    Configurations::precision currentTorque = 0;
    vector<Configurations::precision> driveHandleForces{};

    vector<Series> angularVelocityMatrix{};
    vector<Series> angularAccelerationMatrix{};

    OLSLinearSeries deltaTimes = OLSLinearSeries(Configurations::impulseDataArrayLength);
    OLSLinearSeries deltaTimesSlopes = OLSLinearSeries(Configurations::impulseDataArrayLength);
    Series deltaTimesSlopesSeries = Series(Configurations::impulseDataArrayLength);
    OLSLinearSeries recoveryDeltaTimes = OLSLinearSeries();
    TSQuadraticSeries angularDistances = TSQuadraticSeries(Configurations::impulseDataArrayLength);

    bool isFlywheelUnpowered() const;
    bool isFlywheelPowered() const;
    void calculateDragCoefficient();
    void calculateAvgStrokePower();

    void driveStart();
    void driveUpdate();
    void driveEnd();
    void recoveryStart();
    void recoveryUpdate();
    void recoveryEnd();

public:
    StrokeService();

    RowingDataModels::RowingMetrics getData();
    void processData(RowingDataModels::FlywheelData data);
};