#pragma once

#include "NimBLEDevice.h"

class ExtendedMetricBleService;

class ServerCallbacks final : public NimBLEServerCallbacks
{
    ExtendedMetricBleService &extendedMetricsBleService;

public:
    explicit ServerCallbacks(ExtendedMetricBleService &_extendedMetricsBleService);

    void onConnect(NimBLEServer *pServer) override;
    void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override;
};