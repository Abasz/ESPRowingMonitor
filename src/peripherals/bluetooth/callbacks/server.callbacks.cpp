
#include <algorithm>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../../utils/configuration.h"
#include "./server.callbacks.h"

ServerCallbacks::ServerCallbacks(IExtendedMetricBleService &_extendedMetricsBleService) : extendedMetricsBleService(_extendedMetricsBleService)
{
}

void ServerCallbacks::onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo)
{
    const auto connectedCount = pServer->getConnectedCount();
    Log.verboseln("Device connected: %d", connectedCount);

    if (connectedCount < Configurations::maxConnectionCount)
    {
        auto *const advertising = NimBLEDevice::getAdvertising();

        Log.verboseln("Advertising restarted %T, %T", advertising->stop(), advertising->start());
    }
}