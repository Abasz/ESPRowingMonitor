#pragma once

#include "NimBLEDevice.h"

class BluetoothController;

class ServerCallbacks final : public NimBLEServerCallbacks
{
    BluetoothController &bleController;

public:
    explicit ServerCallbacks(BluetoothController &_bleController);

    void onConnect(NimBLEServer *pServer) override;
    void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override;
};