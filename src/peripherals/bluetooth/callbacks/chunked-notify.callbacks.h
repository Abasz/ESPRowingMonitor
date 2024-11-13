#pragma once

#include "NimBLEDevice.h"

class ExtendedMetricBleService;

class ChunkedNotifyMetricCallbacks final : public NimBLECharacteristicCallbacks
{
    ExtendedMetricBleService &extendedMetricsBleService;

public:
    explicit ChunkedNotifyMetricCallbacks(ExtendedMetricBleService &_extendedMetricsBleService);

    void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue) override;
};