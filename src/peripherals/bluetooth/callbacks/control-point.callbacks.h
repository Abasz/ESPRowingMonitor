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

    ResponseOpCodes processSdCardLogging(const NimBLEAttValue &message);
    ResponseOpCodes processLogLevel(const NimBLEAttValue &message);
    ResponseOpCodes processDeltaTimeLogging(const NimBLEAttValue &message);
    void processBleServiceChange(const NimBLEAttValue &message, NimBLECharacteristic *pCharacteristic);
    ResponseOpCodes processMachineSettingsChange(const NimBLEAttValue &message);

public:
    explicit ControlPointCallbacks(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService);

    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
};