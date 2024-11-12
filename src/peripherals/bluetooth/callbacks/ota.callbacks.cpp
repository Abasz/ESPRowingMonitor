
#include "NimBLEDevice.h"

#include "../bluetooth.controller.h"
#include "./ota.callbacks.h"

using std::array;

OtaRxCallbacks::OtaRxCallbacks(BluetoothController &_bleController) : bleController(_bleController)
{
}

void OtaRxCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic, ble_gap_conn_desc *desc)
{
    const auto mtu = pCharacteristic->getService()->getServer()->getPeerMTU(desc->conn_handle);

    bleController.otaService.onData(pCharacteristic->getValue(), mtu);
}
