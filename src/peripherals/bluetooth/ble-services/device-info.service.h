#pragma once

#include "./device-info.service.interface.h"

class NimBLEServer;
class NimBLEService;

class DeviceInfoBleService final : public IDeviceInfoBleService
{
public:
    NimBLEService *setup(NimBLEServer *server) const override;
};
