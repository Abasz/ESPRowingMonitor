#pragma once

#include "./configuration.h"
#include "./macros.h"

namespace RowerProfile
{
    struct Defaults
    {
        static constexpr unsigned char impulsesPerRevolution = IMPULSES_PER_REVOLUTION;
        static constexpr float flywheelInertia = FLYWHEEL_INERTIA;
        static constexpr float sprocketRadius = SPROCKET_RADIUS / 100.0F;
        static constexpr float concept2MagicNumber = CONCEPT_2_MAGIC_NUMBER;

        // Sensor signal filter settings
        static constexpr unsigned short rotationDebounceTimeMin = ROTATION_DEBOUNCE_TIME_MIN * 1'000;
        static constexpr unsigned int rowingStoppedThresholdPeriod = ROWING_STOPPED_THRESHOLD_PERIOD * 1'000;

        // Drag factor filter settings
        static constexpr float goodnessOfFitThreshold = GOODNESS_OF_FIT_THRESHOLD;
        static constexpr unsigned int maxDragFactorRecoveryPeriod = MAX_DRAG_FACTOR_RECOVERY_PERIOD * 1'000;
        static constexpr float lowerDragFactorThreshold = LOWER_DRAG_FACTOR_THRESHOLD / 1e6F;
        static constexpr float upperDragFactorThreshold = UPPER_DRAG_FACTOR_THRESHOLD / 1e6F;
        static constexpr unsigned char dragCoefficientsArrayLength = DRAG_COEFFICIENTS_ARRAY_LENGTH;

        // Stroke phase detection filter settings
        static constexpr StrokeDetectionType strokeDetectionType = STROKE_DETECTION;
        static constexpr float minimumPoweredTorque = MINIMUM_POWERED_TORQUE;
        static constexpr float minimumDragTorque = MINIMUM_DRAG_TORQUE;
        static constexpr float minimumRecoverySlopeMargin = MINIMUM_RECOVERY_SLOPE_MARGIN / 1e6F;
        static constexpr float minimumRecoverySlope = MINIMUM_RECOVERY_SLOPE;
        static constexpr unsigned int minimumRecoveryTime = MINIMUM_RECOVERY_TIME * 1'000;
        static constexpr unsigned int minimumDriveTime = MINIMUM_DRIVE_TIME * 1'000;
        static constexpr unsigned char impulseDataArrayLength = IMPULSE_DATA_ARRAY_LENGTH;
        static constexpr unsigned char driveHandleForcesMaxCapacity = DRIVE_HANDLE_FORCES_MAX_CAPACITY;
    };

    struct MachineSettings
    {
        unsigned char impulsesPerRevolution = Defaults::impulsesPerRevolution;
        float flywheelInertia = Defaults::flywheelInertia;
        float concept2MagicNumber = Defaults::concept2MagicNumber;
        float sprocketRadius = Defaults::sprocketRadius;
    };

    struct SensorSignalSettings
    {
        unsigned short rotationDebounceTimeMin = Defaults::rotationDebounceTimeMin;
        unsigned int rowingStoppedThresholdPeriod = Defaults::rowingStoppedThresholdPeriod;
    };

    struct DragFactorSettings
    {
        float goodnessOfFitThreshold = Defaults::goodnessOfFitThreshold;
        unsigned int maxDragFactorRecoveryPeriod = Defaults::maxDragFactorRecoveryPeriod;
        float lowerDragFactorThreshold = Defaults::lowerDragFactorThreshold;
        float upperDragFactorThreshold = Defaults::upperDragFactorThreshold;
        unsigned char dragCoefficientsArrayLength = Defaults::dragCoefficientsArrayLength;
    };

    struct StrokePhaseDetectionSettings
    {
        StrokeDetectionType strokeDetectionType = Defaults::strokeDetectionType;
        float minimumPoweredTorque = Defaults::minimumPoweredTorque;
        float minimumDragTorque = Defaults::minimumDragTorque;
        float minimumRecoverySlopeMargin = Defaults::minimumRecoverySlopeMargin;
        float minimumRecoverySlope = Defaults::minimumRecoverySlope;
        unsigned int minimumRecoveryTime = Defaults::minimumRecoveryTime;
        unsigned int minimumDriveTime = Defaults::minimumDriveTime;
        unsigned char impulseDataArrayLength = Defaults::impulseDataArrayLength;
        unsigned char driveHandleForcesMaxCapacity = Defaults::driveHandleForcesMaxCapacity;
    };
}