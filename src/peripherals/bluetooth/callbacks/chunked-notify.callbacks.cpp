
#include "NimBLEDevice.h"

#include "../../../utils/enums.h"
#include "./chunked-notify.callbacks.h"

ChunkedNotifyMetricCallbacks::ChunkedNotifyMetricCallbacks(IExtendedMetricBleService &_extendedMetricsBleService) : extendedMetricsBleService(_extendedMetricsBleService)
{
}

void ChunkedNotifyMetricCallbacks::onSubscribe(NimBLECharacteristic *const pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue)
{
    if (pCharacteristic->getUUID().toString() == CommonBleFlags::handleForcesUuid)
    {
        extendedMetricsBleService.addHandleForcesClientId(desc->conn_handle);
    }
    if (pCharacteristic->getUUID().toString() == CommonBleFlags::deltaTimesUuid)
    {
        extendedMetricsBleService.addDeltaTimesClientId(desc->conn_handle);
    }
}