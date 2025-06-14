
#include "NimBLEDevice.h"

#include "../../../utils/enums.h"
#include "./subscription-manager.callbacks.h"

SubscriptionManagerCallbacks::SubscriptionManagerCallbacks()
{
    clientIds.reserve(Configurations::maxConnectionCount);
}

void SubscriptionManagerCallbacks::onSubscribe(NimBLECharacteristic *const pCharacteristic, NimBLEConnInfo &connInfo, unsigned short subValue)
{
    if (subValue > 0)
    {
        clientIds.push_back(connInfo.getConnHandle());

        return;
    }

    clientIds.erase(
        std::remove_if(
            begin(clientIds),
            end(clientIds),
            [&](unsigned char connectionId)
            {
                return connectionId == connInfo.getConnHandle();
            }),
        cend(clientIds));
}

const vector<unsigned char> &SubscriptionManagerCallbacks::getClientIds() const
{
    return clientIds;
}