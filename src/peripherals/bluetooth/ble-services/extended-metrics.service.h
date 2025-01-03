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

        Configurations::precision avgStrokePower = 0;
        unsigned int recoveryDuration = 0;
        unsigned int driveDuration = 0;
        Configurations::precision dragCoefficient = 0;

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

public:
    ExtendedMetricBleService();

    NimBLEService *setup(NimBLEServer *server) override;

    const vector<unsigned char> &getHandleForcesClientIds() const override;
    void addHandleForcesClientId(unsigned char clientId) override;
    const vector<unsigned char> &getDeltaTimesClientIds() const override;
    void addDeltaTimesClientId(unsigned char clientId) override;

    unsigned short calculateMtu(const std::vector<unsigned char> &clientIds) const override;

    void broadcastHandleForces(const std::vector<float> &handleForces) override;
    void broadcastDeltaTimes(const std::vector<unsigned long> &deltaTimes) override;
    void broadcastExtendedMetrics(Configurations::precision avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, Configurations::precision dragCoefficient) override;

    unsigned char removeDeltaTimesClient(unsigned char clientId) override;
    unsigned char removeHandleForcesClient(unsigned char clientId) override;

    bool isExtendedMetricsSubscribed() const override;
};
