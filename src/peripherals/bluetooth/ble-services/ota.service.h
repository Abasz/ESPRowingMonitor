#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/ota-updater/ota-updater.service.interface.h"
#include "../callbacks/ota.callbacks.h"

class OtaBleService
{
public:
    OtaRxCallbacks callbacks;
    NimBLECharacteristic *characteristic = nullptr;

    explicit OtaBleService(IOtaUploaderService &_otaService);

    NimBLEService *setup(NimBLEServer *server);
};
