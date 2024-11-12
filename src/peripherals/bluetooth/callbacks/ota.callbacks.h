#pragma once

#include "NimBLEDevice.h"

class BluetoothController;

class OtaRxCallbacks final : public NimBLECharacteristicCallbacks
{
    BluetoothController &bleController;

public:
    explicit OtaRxCallbacks(BluetoothController &_bleController);

    void onWrite(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc) override;
};