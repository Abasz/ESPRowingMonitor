#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../../utils/enums.h"
#include "../ble-services/settings.service.interface.h"
#include "../bluetooth.controller.interface.h"

class ControlPointCallbacks final : public NimBLECharacteristicCallbacks
{
    ISettingsBleService &settingsBleService;
    IEEPROMService &eepromService;

    ResponseOpCodes processSdCardLogging(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    ResponseOpCodes processLogLevel(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    ResponseOpCodes processDeltaTimeLogging(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    void processBleServiceChange(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);

public:
    explicit ControlPointCallbacks(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService);

    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
};