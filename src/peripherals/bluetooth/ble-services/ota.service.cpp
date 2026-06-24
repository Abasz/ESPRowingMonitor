#include <string>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "./ota.service.h"

#include "../ble-metrics.model.h"
#include "../ble.enums.h"
#include "../callbacks/ota.callbacks.h"

OtaBleService::OtaBleService(IOtaUpdaterService &_otaService) : callbacks(_otaService)
{
}

NimBLEService *OtaBleService::setup(NimBLEServer *const server)
{
    Log.traceln("Setting up OTA Service");
    auto *const otaBleService = server->createService(std::string{CommonBleFlags::otaServiceUuid});
    txCharacteristic = otaBleService->createCharacteristic(std::string{CommonBleFlags::otaTxUuid}, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);

    otaBleService->createCharacteristic(std::string{CommonBleFlags::otaRxUuid}, NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::WRITE)->setCallbacks(&callbacks);

    return otaBleService;
}

NimBLECharacteristic *OtaBleService::getOtaTx() const
{
    ASSERT_SETUP_CALLED(txCharacteristic);

    return txCharacteristic;
}