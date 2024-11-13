#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../bluetooth.controller.interface.h"
#include "../callbacks/control-point.callbacks.h"

class SettingsBleService
{
public:
    static constexpr unsigned char settingsArrayLength = 1U;

    ControlPointCallbacks callbacks;
    NimBLECharacteristic *characteristic = nullptr;

    explicit SettingsBleService(IBluetoothController &_bleController, IEEPROMService &_eepromService);

    NimBLEService *setup(NimBLEServer *server, std::array<unsigned char, settingsArrayLength> settings);
};
