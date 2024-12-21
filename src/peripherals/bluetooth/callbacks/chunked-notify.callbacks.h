#pragma once

#include <vector>

#include "NimBLEDevice.h"

#include "../../../utils/configuration.h"
#include "../ble-services/extended-metrics.service.interface.h"

class ChunkedNotifyMetricCallbacks final : public NimBLECharacteristicCallbacks
{
    std::vector<unsigned char> clientIds;

public:
    explicit ChunkedNotifyMetricCallbacks();

    void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue) override;

    const vector<unsigned char> &getClientIds() const;
};