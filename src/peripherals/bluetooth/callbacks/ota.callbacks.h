#pragma once

#include "../../../utils/ota-updater/ota-updater.service.interface.h"
#include "NimBLEDevice.h"

class OtaRxCallbacks final : public NimBLECharacteristicCallbacks
{
    IOtaUpdaterService &otaService;

public:
    explicit OtaRxCallbacks(IOtaUpdaterService &_otaService);

    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
};