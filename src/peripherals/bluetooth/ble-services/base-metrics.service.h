#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../../utils/enums.h"
#include "../bluetooth.controller.interface.h"
#include "../callbacks/control-point.callbacks.h"
#include "./base-metrics.service.interface.h"

class BaseMetricsBleService final : public IBaseMetricsBleService
{
    ControlPointCallbacks callbacks;

    struct BaseMetricsParams
    {
        NimBLECharacteristic *characteristic = nullptr;

        unsigned short revTime = 0;
        unsigned int revCount = 0;
        unsigned short strokeTime = 0;
        unsigned short strokeCount = 0;
        short avgStrokePower = 0;
    } parameters;

    NimBLEService *setupCscServices(NimBLEServer *server);
    NimBLEService *setupPscServices(NimBLEServer *server);

    static void pscTask(void *parameters);
    static void cscTask(void *parameters);
    static void (*broadcastTask)(void *);

public:
    explicit BaseMetricsBleService(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService);

    NimBLEService *setup(NimBLEServer *server, BleServiceFlag bleServiceFlag) override;

    void broadcastBaseMetrics(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) override;

    bool isSubscribed() override;
};