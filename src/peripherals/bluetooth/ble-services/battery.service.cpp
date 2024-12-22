#include <array>

#include "ArduinoLog.h"

#include "../../../utils/enums.h"
#include "../ble-metrics.model.h"
#include "./battery.service.h"
#include "battery.service.h"

NimBLEService *BatteryBleService::setup(NimBLEServer *const server)
{
    auto *batteryService = server->createService(CommonBleFlags::batterySvcUuid);

    characteristic = batteryService->createCharacteristic(CommonBleFlags::batteryLevelUuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    return batteryService;
}

void BatteryBleService::broadcastBatteryLevel() const
{
    ASSERT_SETUP_CALLED(characteristic);

    characteristic->notify();
}

void BatteryBleService::setBatteryLevel(unsigned char batteryLevel) const
{
    ASSERT_SETUP_CALLED(characteristic);

    characteristic->setValue(batteryLevel);
}
