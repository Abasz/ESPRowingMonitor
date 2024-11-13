#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../../utils/enums.h"
#include "../bluetooth.controller.interface.h"

class ControlPointCallbacks final : public NimBLECharacteristicCallbacks
{
    IBluetoothController &bleController;
    IEEPROMService &eepromService;

    ResponseOpCodes processSdCardLogging(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    ResponseOpCodes processLogLevel(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    ResponseOpCodes processDeltaTimeLogging(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    void processBleServiceChange(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);

public:
    explicit ControlPointCallbacks(IBluetoothController &_bleController, IEEPROMService &_eepromService);

    void onWrite(NimBLECharacteristic *pCharacteristic) override;
};