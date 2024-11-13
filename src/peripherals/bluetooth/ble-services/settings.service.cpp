#include <array>

#include "ArduinoLog.h"

#include "../../../utils/enums.h"
#include "./settings.service.h"

SettingsBleService::SettingsBleService(IBluetoothController &_bleController, IEEPROMService &_eepromService) : callbacks(_bleController, _eepromService)
{
}

NimBLEService *SettingsBleService::setup(NimBLEServer *const server, const std::array<unsigned char, settingsArrayLength> settings)
{
    Log.traceln("Setting up Settings Service");
    auto *settingsService = server->createService(CommonBleFlags::settingsServiceUuid);
    characteristic = settingsService->createCharacteristic(CommonBleFlags::settingsUuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    characteristic->setValue(settings);

    settingsService->createCharacteristic(CommonBleFlags::settingsControlPointUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE)->setCallbacks(&callbacks);

    return settingsService;
}