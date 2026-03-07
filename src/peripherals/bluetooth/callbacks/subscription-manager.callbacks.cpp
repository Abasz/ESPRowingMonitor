#include <algorithm>
#include <vector>

#include "NimBLEDevice.h"

#include "./subscription-manager.callbacks.h"

#include "../../../utils/configuration.h"

SubscriptionManagerCallbacks::SubscriptionManagerCallbacks()
{
    clientIds.reserve(Configurations::maxConnectionCount);
}

void SubscriptionManagerCallbacks::onSubscribe([[maybe_unused]] NimBLECharacteristic *const pCharacteristic, NimBLEConnInfo &connInfo, unsigned short subValue)
{
    if (subValue > 0)
    {
        clientIds.push_back(connInfo.getConnHandle());

        return;
    }

    const auto [first, last] = std::ranges::remove_if(clientIds, [&](unsigned char connectionId)
                                                      { return connectionId == connInfo.getConnHandle(); });
    clientIds.erase(
        first,
        last);
}

const std::vector<unsigned char> &SubscriptionManagerCallbacks::getClientIds() const
{
    return clientIds;
}