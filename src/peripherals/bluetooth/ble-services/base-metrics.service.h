#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/enums.h"
#include "../callbacks/control-point.callbacks.h"

class BluetoothController;

class BaseMetricsBleService
{
    NimBLEService *setupCscServices(NimBLEServer *server);
    NimBLEService *setupPscServices(NimBLEServer *server);

public:
    ControlPointCallbacks callbacks;

    explicit BaseMetricsBleService(BluetoothController &_bleController);

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