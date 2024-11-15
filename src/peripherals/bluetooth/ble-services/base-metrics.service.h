#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../../utils/enums.h"
#include "../bluetooth.controller.interface.h"
#include "../callbacks/control-point.callbacks.h"

class BaseMetricsBleService
{
    NimBLEService *setupCscServices(NimBLEServer *server);
    NimBLEService *setupPscServices(NimBLEServer *server);

public:
    ControlPointCallbacks callbacks;

    explicit BaseMetricsBleService(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService);

    struct BaseMetricsParams
    {
        NimBLECharacteristic *characteristic = nullptr;

        unsigned short revTime = 0;
        unsigned int revCount = 0;
        unsigned short strokeTime = 0;
        unsigned short strokeCount = 0;
        short avgStrokePower = 0;
    } parameters;

    static void pscTask(void *parameters);
    static void cscTask(void *parameters);

    NimBLEService *setup(NimBLEServer *server, BleServiceFlag bleServiceFlag);
};