#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/ota-updater.service.interface.h"
#include "../callbacks/ota.callbacks.h"

class BluetoothController;

class OtaBleService
{
public:
    OtaRxCallbacks callbacks;
    NimBLECharacteristic *characteristic = nullptr;

    explicit OtaBleService(BluetoothController &_bleController);

    NimBLEService *setup(NimBLEServer *server);
};
