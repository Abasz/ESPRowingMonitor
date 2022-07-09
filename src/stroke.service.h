#pragma once

#include <array>

#include "regressor.service.h"
#include "settings.h"
#include "stroke.model.h"

enum class CyclePhase
{
    Stopped,
    Recovery,
    Drive
};

class StrokeService
{
    LinearRegressorService &regressorService;

    static double constexpr angularDisplacementPerImpulse = (2 * PI) / Settings::impulsesPerRevolution;
    static byte const strokeCycleStartIndex = Settings::deltaTimeArrayLength - Settings::flywheelPowerChangeDetectionErrorThreshold - 1;

    volatile unsigned long lastRevTime = 0;
    volatile unsigned long lastStrokeTime = 0;
    volatile unsigned int impulseCount = 0;
    volatile unsigned int revCount = 0;
    volatile unsigned short strokeCount = 0;

    volatile double avgStrokePower = 0;
    volatile double dragCoefficient = 0;
    volatile unsigned int lastDriveDuration = 0;

    volatile unsigned long driveStartTime = 0;
    volatile unsigned int driveStartImpulseCount = 0;
    volatile unsigned int driveDuration = 0;

    volatile unsigned long recoveryStartTime = 0;
    volatile unsigned int recoveryDuration = 0;

    volatile unsigned long previousDeltaTime = 0;
    volatile unsigned long previousCleanRevTime = 0;
    volatile unsigned long previousRawRevTime = 0;
    volatile unsigned long lastDataReadTime = 0;

    volatile CyclePhase cyclePhase = CyclePhase::Stopped;
    std::array<volatile unsigned long, Settings::deltaTimeArrayLength> cleanDeltaTimes{};
    std::array<volatile unsigned long, Settings::deltaTimeArrayLength> rawDeltaTimes{};
    std::array<double, Settings::dragCoefficientsArrayLength> dragCoefficients{};

    bool isFlywheelUnpowered() const;
    bool isFlywheelPowered() const;
    void calculateDragCoefficient();
    void calculateAvgStrokePower();

public:
    StrokeService(LinearRegressorService &_regressorService);

    void setup() const;
    bool hasDataChanged();
    StrokeModel::CscData getData() const;
    void processRotation(unsigned long now);
};