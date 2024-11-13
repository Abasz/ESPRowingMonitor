#pragma once

#include "NimBLEDevice.h"

#include "../callbacks/chunked-notify.callbacks.h"

using std::vector;

class ExtendedMetricBleService
{
public:
    ChunkedNotifyMetricCallbacks callbacks;

    ExtendedMetricBleService();

    struct ExtendedMetricsParams
    {
        NimBLECharacteristic *characteristic = nullptr;
        short avgStrokePower = 0;
        unsigned short recoveryDuration = 0;
        unsigned short driveDuration = 0;
        unsigned char dragFactor = 0;

        static void task(void *parameters);

    } extendedMetricsParams;

    struct HandleForcesParams
    {
        NimBLECharacteristic *characteristic = nullptr;
        unsigned short mtu = 512U;
        vector<float> handleForces;
        vector<unsigned char> clientIds;

        static void task(void *parameters);

    } handleForcesParams;

    struct DeltaTimesParams
    {
        NimBLECharacteristic *characteristic = nullptr;
        vector<unsigned long> deltaTimes;
        vector<unsigned char> clientIds;

        static void task(void *parameters);

    } deltaTimesParams;

    NimBLEService *setup(NimBLEServer *server);

    const vector<unsigned char> &getHandleForcesClientIds() const;
    void addHandleForcesClientId(unsigned char clientId);
    const vector<unsigned char> &getDeltaTimesClientIds() const;
    void addDeltaTimesClientId(unsigned char clientId);

    unsigned short getDeltaTimesMTU(unsigned char clientId) const;
    unsigned short getHandleForcesMTU(unsigned char clientId) const;

    unsigned char removeDeltaTimesClient(unsigned char clientId);
    unsigned char removeHandleForcesClient(unsigned char clientId);
};
