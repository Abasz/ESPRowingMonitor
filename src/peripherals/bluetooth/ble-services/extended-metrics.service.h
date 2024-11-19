#pragma once

#include <vector>

#include "NimBLEDevice.h"

#include "../callbacks/chunked-notify.callbacks.h"
#include "./extended-metrics.service.interface.h"

using std::vector;

class ExtendedMetricBleService final : public IExtendedMetricBleService
{
    ChunkedNotifyMetricCallbacks callbacks;

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
        unsigned short chunkSize = (512U - 3 - 2) / sizeof(float);
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

    static unsigned short calculateHandleForcesChunkSize(unsigned short mtu);
    unsigned short getClientHandleForcesMtu(unsigned char clientId) const;

public:
    ExtendedMetricBleService();

    NimBLEService *setup(NimBLEServer *server) override;

    const vector<unsigned char> &getHandleForcesClientIds() const override;
    void addHandleForcesClientId(unsigned char clientId) override;
    const vector<unsigned char> &getDeltaTimesClientIds() const override;
    void addDeltaTimesClientId(unsigned char clientId) override;

    unsigned short getDeltaTimesClientMtu(unsigned char clientId) const override;

    void broadcastHandleForces(const std::vector<float> &handleForces) override;
    void broadcastDeltaTimes(const std::vector<unsigned long> &deltaTimes) override;
    void broadcastExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor) override;

    unsigned char removeDeltaTimesClient(unsigned char clientId) override;
    unsigned char removeHandleForcesClient(unsigned char clientId) override;

    bool isExtendedMetricsSubscribed() const override;
};
