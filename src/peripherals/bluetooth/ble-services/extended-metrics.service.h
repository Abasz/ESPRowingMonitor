#pragma once

#include <vector>

#include "../callbacks/subscription-manager.callbacks.h"
#include "./extended-metrics.service.interface.h"
#include "../../../utils/configuration.h"

class NimBLECharacteristic;
class NimBLEServer;
class NimBLEService;

using std::vector;

class ExtendedMetricBleService final : public IExtendedMetricBleService
{
    struct ExtendedMetricsParams
    {
        NimBLECharacteristic *characteristic = nullptr;
        SubscriptionManagerCallbacks callbacks;

        Configurations::precision avgStrokePower = 0;
        unsigned int recoveryDuration = 0;
        unsigned int driveDuration = 0;
        Configurations::precision dragCoefficient = 0;

        static void task(void *parameters);

    } extendedMetricsParams;

    struct HandleForcesParams
    {
        NimBLECharacteristic *characteristic = nullptr;
        SubscriptionManagerCallbacks callbacks;

        unsigned short chunkSize = (512U - 3 - 2) / sizeof(float);
        vector<float> handleForces;

        static void task(void *parameters);

    } handleForcesParams;

    struct DeltaTimesParams
    {
        SubscriptionManagerCallbacks callbacks;
        NimBLECharacteristic *characteristic = nullptr;
        vector<unsigned long> deltaTimes;

        static void task(void *parameters);

    } deltaTimesParams;

public:
    NimBLEService *setup(NimBLEServer *server) override;

    [[nodiscard]] const vector<unsigned char> &getHandleForcesClientIds() const override;
    [[nodiscard]] const vector<unsigned char> &getDeltaTimesClientIds() const override;
    [[nodiscard]] const vector<unsigned char> &getExtendedMetricsClientIds() const override;

    [[nodiscard]] unsigned short calculateMtu(const std::vector<unsigned char> &clientIds) const override;

    void broadcastHandleForces(const std::vector<float> &handleForces) override;
    void broadcastDeltaTimes(const std::vector<unsigned long> &deltaTimes) override;
    void broadcastExtendedMetrics(Configurations::precision avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, Configurations::precision dragCoefficient) override;
};
