
#include "NimBLEDevice.h"

#include "../../../utils/enums.h"
#include "./connection-manager.callbacks.h"

ConnectionManagerCallbacks::ConnectionManagerCallbacks()
{
    clientIds.reserve(Configurations::maxConnectionCount);
}

void ConnectionManagerCallbacks::onSubscribe(NimBLECharacteristic *const pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue)
{
    if (subValue > 0)
    {
        clientIds.push_back(desc->conn_handle);

        return;
    }

    clientIds.erase(
        std::remove_if(
            begin(clientIds),
            end(clientIds),
            [&](unsigned char connectionId)
            {
                return connectionId == desc->conn_handle;
            }),
        cend(clientIds));
}

const vector<unsigned char> &ConnectionManagerCallbacks::getClientIds() const
{
    return clientIds;
}