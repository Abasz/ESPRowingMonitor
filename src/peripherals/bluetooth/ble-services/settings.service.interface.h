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

    static constexpr unsigned char baseSettingsPayloadSize = 1U;

    static constexpr unsigned char flywheelInertiaPayloadSize = 4U;
    static constexpr unsigned char magicNumberPayloadSize = 1U;
    static constexpr unsigned char machineSettingsPayloadSize = flywheelInertiaPayloadSize + magicNumberPayloadSize;

    static constexpr unsigned char settingsPayloadSize = baseSettingsPayloadSize + machineSettingsPayloadSize;

    virtual NimBLEService *setup(NimBLEServer *server) = 0;
    virtual void broadcastSettings() const = 0;
};