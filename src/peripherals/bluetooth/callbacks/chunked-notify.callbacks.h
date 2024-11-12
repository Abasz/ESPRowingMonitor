#pragma once

#include "NimBLEDevice.h"

class BluetoothController;

class ChunkedNotifyMetricCallbacks final : public NimBLECharacteristicCallbacks
{
    BluetoothController &bleController;

public:
    explicit ChunkedNotifyMetricCallbacks(BluetoothController &_bleController);

    void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue) override;
};