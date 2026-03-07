#pragma once

#include <vector>

#include "NimBLEDevice.h"

class SubscriptionManagerCallbacks final : public NimBLECharacteristicCallbacks
{
    std::vector<unsigned char> clientIds;

public:
    SubscriptionManagerCallbacks();

    void onSubscribe(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo, unsigned short subValue) override;

    [[nodiscard]] const std::vector<unsigned char> &getClientIds() const;
};