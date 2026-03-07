#pragma once

#include "./connection-manager.callbacks.interface.h"

class NimBLEConnInfo;
class NimBLEServer;

class ConnectionManagerCallbacks final : public IConnectionManagerCallbacks
{
    unsigned char connectionCount = 0;

public:
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override;
    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override;

    [[nodiscard]] unsigned char getConnectionCount() const override;
};