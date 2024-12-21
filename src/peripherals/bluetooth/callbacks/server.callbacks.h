#pragma once

#include "NimBLEDevice.h"

#include "../ble-services/extended-metrics.service.interface.h"

class ServerCallbacks final : public NimBLEServerCallbacks
{
    IExtendedMetricBleService &extendedMetricsBleService;

public:
    explicit ServerCallbacks(IExtendedMetricBleService &_extendedMetricsBleService);

    void onConnect(NimBLEServer *pServer) override;
};