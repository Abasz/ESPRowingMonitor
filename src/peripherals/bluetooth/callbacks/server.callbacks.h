#pragma once

#include "NimBLEDevice.h"

#include "./server.callbacks.interface.h"

class ServerCallbacks final : public IServerCallbacks
{
    unsigned char connectionCount = 0;

public:
    explicit ServerCallbacks();

    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override;
    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override;

    unsigned char getConnectionCount() const override;
};