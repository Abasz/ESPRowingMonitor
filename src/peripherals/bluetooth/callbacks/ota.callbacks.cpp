
#include "NimBLEDevice.h"

#include "../bluetooth.controller.h"
#include "./ota.callbacks.h"

using std::array;

OtaRxCallbacks::OtaRxCallbacks(IOtaUploaderService &_otaService) : otaService(_otaService)
{
}

void OtaRxCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic, ble_gap_conn_desc *desc)
{
    const auto mtu = pCharacteristic->getService()->getServer()->getPeerMTU(desc->conn_handle);

    otaService.onData(pCharacteristic->getValue(), mtu);
}
