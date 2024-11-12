
#include "NimBLEDevice.h"

#include "../../../utils/enums.h"
#include "../bluetooth.controller.h"
#include "./chunked-notify.callbacks.h"

ChunkedNotifyMetricCallbacks::ChunkedNotifyMetricCallbacks(BluetoothController &_bleController) : bleController(_bleController)
{
}

void ChunkedNotifyMetricCallbacks::onSubscribe(NimBLECharacteristic *const pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue)
{
    if (pCharacteristic->getUUID().toString() == CommonBleFlags::handleForcesUuid)
    {
        bleController.extendedMetricsBleService.handleForcesParams.clientIds.push_back(desc->conn_handle);
    }
    if (pCharacteristic->getUUID().toString() == CommonBleFlags::deltaTimesUuid)
    {
        bleController.extendedMetricsBleService.deltaTimesParams.clientIds.push_back(desc->conn_handle);
    }
}