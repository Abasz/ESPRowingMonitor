#pragma once

#include "./device-info.service.interface.h"

class DeviceInfoBleService final : public IDeviceInfoBleService
{
public:
    NimBLEService *setup(NimBLEServer *server) const override;
};
