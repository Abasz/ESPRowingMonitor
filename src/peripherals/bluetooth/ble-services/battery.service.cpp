#include <array>

#include "ArduinoLog.h"

#include "../../../utils/enums.h"
#include "./battery.service.h"

NimBLEService *BatteryBleService::setup(NimBLEServer *const server)
{
    auto *batteryService = server->createService(CommonBleFlags::batterySvcUuid);

    characteristic = batteryService->createCharacteristic(CommonBleFlags::batteryLevelUuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    return batteryService;
}