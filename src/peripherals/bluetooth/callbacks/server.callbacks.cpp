
#include <algorithm>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../../utils/configuration.h"
#include "./server.callbacks.h"

ServerCallbacks::ServerCallbacks(IExtendedMetricBleService &_extendedMetricsBleService) : extendedMetricsBleService(_extendedMetricsBleService)
{
}

void ServerCallbacks::onConnect(NimBLEServer *pServer)
{
    if (pServer->getConnectedCount() < Configurations::maxConnectionCount)
    {
        Log.verboseln("Device connected");
        NimBLEDevice::getAdvertising()->start();
    }
}