#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/ota-updater/ota-updater.service.interface.h"
#include "../callbacks/ota.callbacks.h"
#include "./ota.service.interface.h"

class OtaBleService final : public IOtaBleService
{
    OtaRxCallbacks callbacks;
    NimBLECharacteristic *txCharacteristic = nullptr;

public:
    explicit OtaBleService(IOtaUpdaterService &_otaService);

    NimBLEService *setup(NimBLEServer *server) override;

    NimBLECharacteristic *getOtaTx() const override;
};
