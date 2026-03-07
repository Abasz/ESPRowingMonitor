#pragma once

#include <vector>

#include "../ble-metrics.model.h"
#include "../callbacks/control-point.callbacks.h"
#include "../callbacks/subscription-manager.callbacks.h"
#include "./base-metrics.service.interface.h"

class IEEPROMService;
class ISettingsBleService;
class NimBLECharacteristic;
class NimBLEServer;
class NimBLEService;
enum class BleServiceFlag : unsigned char;

class BaseMetricsBleService final : public IBaseMetricsBleService
{
    ControlPointCallbacks controlPointCallbacks;
    SubscriptionManagerCallbacks connectionManager;

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

    [[nodiscard]] const std::vector<unsigned char> &getClientIds() const override;
};