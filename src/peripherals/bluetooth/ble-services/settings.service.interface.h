#pragma once

#include <array>

#include "NimBLEDevice.h"

#include "../../../utils/enums.h"

class ISettingsBleService
{
protected:
    ~ISettingsBleService() = default;

public:
    static constexpr auto magicNumberScale = 35.0F;
    static constexpr auto sprocketRadiusScale = 1000.0F;
    static constexpr unsigned short debounceTimeScale = 1'000U;
    static constexpr unsigned int rowingStoppedThresholdScale = 1'000'000U;
    static constexpr auto goodnessOfFitThresholdScale = static_cast<float>(UCHAR_MAX);
    static constexpr unsigned int dragFactorRecoveryPeriodScale = 1'000'000U;
    static constexpr auto dragFactorThresholdScale = 1e6F;
    static constexpr unsigned short poweredTorqueScale = 10'000;
    static constexpr unsigned short dragTorqueScale = 10'000;
    static constexpr float recoverySlopeMarginPayloadScale = 1e6F;
    static constexpr unsigned short recoverySlopeScale = 1'000U;
    static constexpr unsigned int minimumStrokeTimesScale = 1'000;

    static constexpr unsigned char baseSettingsPayloadSize = 1U;

    static constexpr unsigned char flywheelInertiaPayloadSize = 4U;
    static constexpr unsigned char magicNumberPayloadSize = 1U;
    static constexpr unsigned char sprocketRadiusPayloadSize = 2U;
    static constexpr unsigned char impulsesPerRevolutionPayloadSize = 1U;
    static constexpr unsigned char machineSettingsPayloadSize = flywheelInertiaPayloadSize + magicNumberPayloadSize + sprocketRadiusPayloadSize + impulsesPerRevolutionPayloadSize;

    static constexpr unsigned char rotationDebouncePayloadSize = 1U;
    static constexpr unsigned char rowingStoppedThresholdPayloadSize = 1U;
    static constexpr unsigned char sensorSignalSettingsPayloadSize = rotationDebouncePayloadSize + rowingStoppedThresholdPayloadSize;

    static constexpr unsigned char goodnessOfFitPayloadSize = 1U;
    static constexpr unsigned char dragFactorRecoveryPeriodPayloadSize = 1U;
    static constexpr unsigned char lowerDragFactorPayloadSize = 2U;
    static constexpr unsigned char upperDragFactorPayloadSize = 2U;
    static constexpr unsigned char dragArrayLengthPayloadSize = 1U;
    static constexpr unsigned char dragFactorSettingsPayloadSize = goodnessOfFitPayloadSize + dragFactorRecoveryPeriodPayloadSize + lowerDragFactorPayloadSize + upperDragFactorPayloadSize + dragArrayLengthPayloadSize;

    static constexpr unsigned char settingsPayloadSize = baseSettingsPayloadSize + machineSettingsPayloadSize + sensorSignalSettingsPayloadSize + dragFactorSettingsPayloadSize;

    static constexpr unsigned char poweredTorquePayloadSize = 2U;
    static constexpr unsigned char dragTorquePayloadSize = 2U;
    static constexpr unsigned char recoverySlopeMarginPayloadSize = 4U;
    static constexpr unsigned char recoverySlopePayloadSize = 2U;
    static constexpr unsigned char minimumStrokeTimesPayloadSize = 3U;
    static constexpr unsigned char impulseAndDetectionTypePayloadSize = 1U;
    static constexpr unsigned char forceCapacityPayloadSize = 1U;

    static constexpr unsigned char strokeSettingsPayloadSize = poweredTorquePayloadSize + dragTorquePayloadSize + recoverySlopeMarginPayloadSize + recoverySlopePayloadSize + minimumStrokeTimesPayloadSize + impulseAndDetectionTypePayloadSize + forceCapacityPayloadSize;

    virtual NimBLEService *setup(NimBLEServer *server) = 0;
    virtual void broadcastSettings() const = 0;
    virtual void broadcastStrokeDetectionSettings() const = 0;
};