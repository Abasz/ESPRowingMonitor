#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../../utils/enums.h"
#include "../ble-metrics.model.h"
#include "../bluetooth.controller.interface.h"
#include "../callbacks/connection-manager.callbacks.h"
#include "../callbacks/control-point.callbacks.h"
#include "./base-metrics.service.interface.h"

class BaseMetricsBleService final : public IBaseMetricsBleService
{
    ControlPointCallbacks controlPointCallbacks;
    ConnectionManagerCallbacks connectionManager;

    struct BaseMetricsParams
    {
        NimBLECharacteristic *characteristic = nullptr;

        BleMetricsModel::BleMetricsData data = {};
    } parameters;

    NimBLEService *setupCscServices(NimBLEServer *server);
    NimBLEService *setupPscServices(NimBLEServer *server);
    NimBLEService *setupFtmsServices(NimBLEServer *server);

    static void pscTask(void *parameters);
    static void cscTask(void *parameters);
    static void ftmsTask(void *parameters);

    static void (*broadcastTask)(void *);

public:
    explicit BaseMetricsBleService(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService);

    NimBLEService *setup(NimBLEServer *server, BleServiceFlag bleServiceFlag) override;

    void broadcastBaseMetrics(const BleMetricsModel::BleMetricsData &data) override;

    const vector<unsigned char> &getClientIds() const override;
};