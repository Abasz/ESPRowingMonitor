#pragma once

#include <vector>

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../../utils/enums.h"
#include "../ble-metrics.model.h"
#include "../bluetooth.controller.interface.h"
#include "../callbacks/control-point.callbacks.h"

class IBaseMetricsBleService
{
protected:
    ~IBaseMetricsBleService() = default;

public:
    virtual NimBLEService *setup(NimBLEServer *server, BleServiceFlag bleServiceFlag) = 0;

    virtual void broadcastBaseMetrics(const BleMetricsModel::BleMetricsData &data) = 0;

    virtual const std::vector<unsigned char> &getClientIds() const = 0;
};