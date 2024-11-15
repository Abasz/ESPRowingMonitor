#pragma once

#include <array>

#include "NimBLEDevice.h"

#include "../../../utils/enums.h"

class ISettingsBleService
{
protected:
    ~ISettingsBleService() = default;

public:
    static constexpr unsigned char settingsArrayLength = 1U;

    virtual NimBLEService *setup(NimBLEServer *server) = 0;
    virtual void broadcastSettings() const = 0;
};