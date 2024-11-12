#pragma once

#include "NimBLEDevice.h"

#include "../callbacks/chunked-notify.callbacks.h"

using std::vector;

class BluetoothController;

class ExtendedMetricBleService
{
public:
    ChunkedNotifyMetricCallbacks callbacks;

    explicit ExtendedMetricBleService(BluetoothController &_bleController);

    struct ExtendedMetricsParams
    {
        NimBLECharacteristic *characteristic = nullptr;
        short avgStrokePower = 0;
        unsigned short recoveryDuration = 0;
        unsigned short driveDuration = 0;
        unsigned char dragFactor = 0;
    } extendedMetricsParams;

    struct HandleForcesParams
    {
        NimBLECharacteristic *characteristic = nullptr;
        unsigned short mtu = 512U;
        vector<float> handleForces;
        vector<unsigned char> clientIds;

    } handleForcesParams;

    struct DeltaTimesParams
    {
        NimBLECharacteristic *characteristic = nullptr;
        vector<unsigned long> deltaTimes;
        vector<unsigned char> clientIds;

    } deltaTimesParams;

    static void extendedMetricsTask(void *parameters);
    static void handleForcesTask(void *parameters);
    static void deltaTimesTask(void *parameters);

    NimBLEService *setup(NimBLEServer *server);
};
