#pragma once

#include <array>

#include "stroke.model.h"

enum class CyclePhase
{
    Stopped,
    Recovery,
    Drive
};

class StrokeService
{
    static byte const ROTATION_DEBOUNCE_TIME_MIN = 15;
    static byte const STROKE_DEBOUNCE_TIME = 200;

    static unsigned short const ROWING_STOPPED_THRESHOLD_PERIOD = 7000;
    static unsigned short const MAX_DRAG_FACTOR_RECOVERY_PERIOD = 6000;
    static double constexpr LOWER_DRAG_FACTOR_THRESHOLD = 75 / 1e6;
    static double constexpr UPPER_DRAG_FACTOR_THRESHOLD = 250 / 1e6;
    static double constexpr FLYWHEEL_INERTIA = 0.073;

    static byte const FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD = 1;
    static byte const DELTA_TIME_ARRAY_LENGTH = 2;
    static byte const DRAG_COEFFICIENTS_ARRAY_LENGTH = 1;

    volatile unsigned long lastRevTime = 0;
    volatile unsigned long lastStrokeTime = 0;
    volatile unsigned int revCount = 0;
    volatile unsigned short strokeCount = 0;

    volatile double avgStrokePower = 0;
    volatile double dragCoefficient = 0;
    volatile unsigned int lastDriveDuration = 0;

    volatile unsigned long driveStartTime = 0;
    volatile unsigned int driveStartRevCount = 0;
    volatile unsigned int driveDuration = 0;

    volatile double recoveryStartAngularVelocity = 0.0;
    volatile unsigned long recoveryStartTime = 0;
    volatile unsigned int recoveryStartRevCount = 0;
    volatile unsigned int recoveryDuration = 0;

    volatile unsigned long previousDeltaTime = 0;
    volatile unsigned long previousRawRevTime = 0;

    volatile CyclePhase cyclePhase = CyclePhase::Stopped;
    std::array<volatile unsigned long, DELTA_TIME_ARRAY_LENGTH> cleanDeltaTimes{};
    std::array<double, DRAG_COEFFICIENTS_ARRAY_LENGTH> dragCoefficients{};

    bool isFlywheelUnpowered() const;
    bool isFlywheelPowered() const;
    void calculateDragCoefficient();
    void calculateAvgStrokePower();

public:
    StrokeService();

    void setup() const;
    StrokeModel::CscData getData() const;
    void processRotation(unsigned long now);
};