#include <algorithm>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../../utils/configuration.h"
#include "./server.callbacks.h"

ServerCallbacks::ServerCallbacks()
{
}

void ServerCallbacks::onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo)
{
    connectionCount = pServer->getConnectedCount();
    Log.verboseln("Device connected, handle: %d, total connections: %d", connInfo.getConnHandle(), connectionCount);

    if (connectionCount < Configurations::maxConnectionCount)
    {
        auto *const advertising = NimBLEDevice::getAdvertising();

        Log.verboseln("Advertising restarted (stoped)%T, (started)%T", advertising->stop(), advertising->start());
    }
}

void ServerCallbacks::onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason)
{
    connectionCount = pServer->getConnectedCount();

    Log.verboseln("Device disconnected, handle: %d, remaining connections: %d", connInfo.getConnHandle(), connectionCount);
}

unsigned char ServerCallbacks::getConnectionCount() const
{
    return connectionCount;
}