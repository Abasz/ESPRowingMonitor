#pragma once

#include "NimBLEDevice.h"

#include "../ble.enums.h"

class IEEPROMService;
class ISettingsBleService;

class ControlPointCallbacks final : public NimBLECharacteristicCallbacks
{
    ISettingsBleService &settingsBleService;
    IEEPROMService &eepromService;

    ResponseOpCodes processSdCardLogging(const NimBLEAttValue &message);
    ResponseOpCodes processLogLevel(const NimBLEAttValue &message);
    ResponseOpCodes processDeltaTimeLogging(const NimBLEAttValue &message);
    void processBleServiceChange(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    ResponseOpCodes processMachineSettingsChange(const NimBLEAttValue &message);
    ResponseOpCodes processSensorSignalSettingsChange(const NimBLEAttValue &message);
    ResponseOpCodes processDragFactorSettingsChange(const NimBLEAttValue &message);
    ResponseOpCodes processStrokeDetectionSettingsChange(const NimBLEAttValue &message);

public:
    explicit ControlPointCallbacks(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService);

    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
};