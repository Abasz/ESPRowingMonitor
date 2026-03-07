#pragma once

#include "../callbacks/ota.callbacks.h"
#include "./ota.service.interface.h"

class IOtaUpdaterService;
class NimBLECharacteristic;

class OtaBleService final : public IOtaBleService
{
    OtaRxCallbacks callbacks;
    NimBLECharacteristic *txCharacteristic = nullptr;

public:
    explicit OtaBleService(IOtaUpdaterService &_otaService);

    NimBLEService *setup(NimBLEServer *server) override;

    [[nodiscard]] NimBLECharacteristic *getOtaTx() const override;
};
