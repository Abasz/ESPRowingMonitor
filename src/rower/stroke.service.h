#pragma once

#include <array>

#include "../settings.h"
#include "../utils/ols-linear-series.h"
#include "../utils/series.h"
#include "../utils/ts-quadratic-series.h"
#include "stroke.model.h"

class StrokeService
{
    static double constexpr angularDisplacementPerImpulse = (2 * PI) / Settings::impulsesPerRevolution;
    static double constexpr sprocketRadius = Settings::sprocketRadius / 100;

    // rower state
    CyclePhase cyclePhase = CyclePhase::Stopped;
    unsigned long long rowingTotalTime = 0ULL;
    unsigned long long rowingImpulseCount = 0UL;
    double rowingTotalAngularDisplacement = 0;

    // Drive related
    unsigned long long driveStartTime = 0ULL;
    unsigned int driveDuration = 0;
    double driveStartAngularDisplacement = 0;
    double driveTotalAngularDisplacement = 0;

    // Recovery related
    unsigned long long recoveryStartTime = 0;
    unsigned int recoveryDuration = 0;
    double recoveryStartAngularDisplacement = 0;
    double recoveryTotalAngularDisplacement = 0;

    // metrics
    double distance = 0;
    unsigned short strokeCount = 0;
    unsigned long long strokeTime = 0ULL;
    unsigned long long revTime = 0ULL;
    double avgStrokePower = 0;

    double dragCoefficient = 0;

    std::array<double, Settings::dragCoefficientsArrayLength> dragCoefficients{};

    double totalAngularDisplacement = 0;

    // advance metrics
    double currentAngularVelocity = 0;
    double currentAngularAcceleration = 0;
    double currentTorque = 0;
    vector<double> driveHandleForces{};

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