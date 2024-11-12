#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/enums.h"

class BluetoothController;

class ControlPointCallbacks final : public NimBLECharacteristicCallbacks
{
    BluetoothController &bleController;

    ResponseOpCodes processSdCardLogging(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    ResponseOpCodes processLogLevel(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    ResponseOpCodes processDeltaTimeLogging(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    void processBleServiceChange(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);

public:
    explicit ControlPointCallbacks(BluetoothController &_bleController);

    void onWrite(NimBLECharacteristic *pCharacteristic) override;
};