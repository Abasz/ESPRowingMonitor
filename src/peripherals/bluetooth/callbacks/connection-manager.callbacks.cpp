#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "./connection-manager.callbacks.h"

#include "../../../utils/configuration.h"

void ConnectionManagerCallbacks::onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo)
{
    connectionCount = pServer->getConnectedCount();
    Log.verboseln("Device connected, handle: %d, total connections: %d", connInfo.getConnHandle(), connectionCount);

    if (connectionCount < Configurations::maxConnectionCount)
    {
        auto *const advertising = NimBLEDevice::getAdvertising();

        Log.verboseln("Advertising restarted (stoped)%T, (started)%T", advertising->stop(), advertising->start());
    }
}

void ConnectionManagerCallbacks::onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, [[maybe_unused]] int reason)
{
    connectionCount = pServer->getConnectedCount();

    Log.verboseln("Device disconnected, handle: %d, remaining connections: %d", connInfo.getConnHandle(), connectionCount);
}

unsigned char ConnectionManagerCallbacks::getConnectionCount() const
{
    return connectionCount;
}