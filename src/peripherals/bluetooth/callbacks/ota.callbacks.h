#pragma once

#include "NimBLEDevice.h"

class IOtaUpdaterService;

class OtaRxCallbacks final : public NimBLECharacteristicCallbacks
{
    IOtaUpdaterService &otaService;

public:
    explicit OtaRxCallbacks(IOtaUpdaterService &_otaService);

    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
};