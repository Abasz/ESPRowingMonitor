#include "NimBLEDevice.h"

#include "./ota.callbacks.h"

#include "../../../utils/ota-updater/ota-updater.service.interface.h"

OtaRxCallbacks::OtaRxCallbacks(IOtaUpdaterService &_otaService) : otaService(_otaService)
{
}

void OtaRxCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic, NimBLEConnInfo &connInfo)
{
    const auto mtu = pCharacteristic->getService()->getServer()->getPeerMTU(connInfo.getConnHandle());

    otaService.onData(pCharacteristic->getValue(), mtu);
}
