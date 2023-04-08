#pragma once

#include <array>

#include "../settings.h"
#include "../utils/ols-linear-series.h"
#include "../utils/series.h"
#include "../utils/ts-quadratic-series.h"
#include "stroke.model.h"

class StrokeService
{
    static Settings::precision constexpr angularDisplacementPerImpulse = (2 * PI) / Settings::impulsesPerRevolution;
    static Settings::precision constexpr sprocketRadius = Settings::sprocketRadius / 100;

    // rower state
    CyclePhase cyclePhase = CyclePhase::Stopped;
    unsigned long long rowingTotalTime = 0ULL;
    unsigned long long rowingImpulseCount = 0UL;
    Settings::precision rowingTotalAngularDisplacement = 0;

    // Drive related
    unsigned long long driveStartTime = 0ULL;
    unsigned int driveDuration = 0;
    Settings::precision driveStartAngularDisplacement = 0;
    Settings::precision driveTotalAngularDisplacement = 0;

    // Recovery related
    unsigned long long recoveryStartTime = 0;
    unsigned int recoveryDuration = 0;
    Settings::precision recoveryStartAngularDisplacement = 0;
    Settings::precision recoveryTotalAngularDisplacement = 0;
    Settings::precision recoveryStartDistance = 0;

    // metrics
    Settings::precision distancePerAngularDisplacement = 0;
    Settings::precision distance = 0;
    unsigned short strokeCount = 0;
    unsigned long long strokeTime = 0ULL;
    unsigned long long revTime = 0ULL;
    Settings::precision avgStrokePower = 0;

    Settings::precision dragCoefficient = 0;

    std::array<Settings::precision, Settings::dragCoefficientsArrayLength> dragCoefficients{};

    Settings::precision totalAngularDisplacement = 0;

    // advance metrics
    Settings::precision currentAngularVelocity = 0;
    Settings::precision currentAngularAcceleration = 0;
    Settings::precision currentTorque = 0;
    vector<Settings::precision> driveHandleForces{};

    vector<Series> angularVelocityMatrix{};
    vector<Series> angularAccelerationMatrix{};

    OLSLinearSeries deltaTimes = OLSLinearSeries(Settings::impulseDataArrayLength);
    OLSLinearSeries deltaTimesSlopes = OLSLinearSeries(Settings::impulseDataArrayLength);
    Series deltaTimesSlopesSeries = Series(Settings::impulseDataArrayLength);
    OLSLinearSeries recoveryDeltaTimes = OLSLinearSeries();
    TSQuadraticSeries angularDistances = TSQuadraticSeries(Settings::impulseDataArrayLength);

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