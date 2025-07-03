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

    static constexpr unsigned char baseSettingsPayloadSize = 1U;

    static constexpr unsigned char flywheelInertiaPayloadSize = 4U;
    static constexpr unsigned char magicNumberPayloadSize = 1U;
    static constexpr unsigned char sprocketRadiusPayloadSize = 2U;
    static constexpr unsigned char impulsesPerRevolutionPayloadSize = 1U;
    static constexpr unsigned char machineSettingsPayloadSize = flywheelInertiaPayloadSize + magicNumberPayloadSize + sprocketRadiusPayloadSize + impulsesPerRevolutionPayloadSize;

    static constexpr unsigned char rotationDebouncePayloadSize = 1U;
    static constexpr unsigned char rowingStoppedThresholdPayloadSize = 1U;
    static constexpr unsigned char sensorSignalSettingsPayloadSize = rotationDebouncePayloadSize + rowingStoppedThresholdPayloadSize;

    static constexpr unsigned char settingsPayloadSize = baseSettingsPayloadSize + machineSettingsPayloadSize + sensorSignalSettingsPayloadSize;

    virtual NimBLEService *setup(NimBLEServer *server) = 0;
    virtual void broadcastSettings() const = 0;
};