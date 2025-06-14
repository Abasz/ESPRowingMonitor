#pragma once

#include "NimBLEDevice.h"

#include "./connection-manager.callbacks.interface.h"

class ConnectionManagerCallbacks final : public IConnectionManagerCallbacks
{
    unsigned char connectionCount = 0;

public:
    explicit ConnectionManagerCallbacks();

    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override;
    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override;

    unsigned char getConnectionCount() const override;
};