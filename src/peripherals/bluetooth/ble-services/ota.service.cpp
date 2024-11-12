#include <array>

#include "ArduinoLog.h"

#include "../../../utils/enums.h"
#include "./ota.service.h"

OtaBleService::OtaBleService(BluetoothController &_bleController) : callbacks(_bleController)
{
}

NimBLEService *OtaBleService::setup(NimBLEServer *const server)
{
    Log.traceln("Setting up OTA Service");
    auto *otaBleService = server->createService(CommonBleFlags::otaServiceUuid);
    characteristic = otaBleService->createCharacteristic(CommonBleFlags::otaTxUuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);

    otaBleService->createCharacteristic(CommonBleFlags::otaRxUuid, NIMBLE_PROPERTY::WRITE)->setCallbacks(&callbacks);

    return otaBleService;
}