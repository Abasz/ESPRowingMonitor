#pragma once

#include "NimBLEDevice.h"

#include "../ble-services/extended-metrics.service.interface.h"

class ChunkedNotifyMetricCallbacks final : public NimBLECharacteristicCallbacks
{
    IExtendedMetricBleService &extendedMetricsBleService;

public:
    explicit ChunkedNotifyMetricCallbacks(IExtendedMetricBleService &_extendedMetricsBleService);

    void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue) override;
};