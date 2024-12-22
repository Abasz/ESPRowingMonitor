#include "NimBLEDevice.h"

#include "../bluetooth.controller.h"
#include "./ota.callbacks.h"

using std::array;

OtaRxCallbacks::OtaRxCallbacks(IOtaUpdaterService &_otaService) : otaService(_otaService)
{
}

void OtaRxCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic, NimBLEConnInfo &connInfo)
{
    const auto mtu = pCharacteristic->getService()->getServer()->getPeerMTU(connInfo.getConnHandle());

    otaService.onData(pCharacteristic->getValue(), mtu);
}
