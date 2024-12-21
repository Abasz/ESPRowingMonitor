#pragma once

#include <vector>

#include "NimBLEDevice.h"

#include "../../../utils/configuration.h"

using std::vector;

class IExtendedMetricBleService
{
protected:
    ~IExtendedMetricBleService() = default;

public:
    virtual NimBLEService *setup(NimBLEServer *server) = 0;

    virtual const vector<unsigned char> &getHandleForcesClientIds() const = 0;
    virtual const vector<unsigned char> &getDeltaTimesClientIds() const = 0;

    virtual unsigned short calculateMtu(const std::vector<unsigned char> &clientIds) const = 0;

    virtual void broadcastHandleForces(const std::vector<float> &handleForces) = 0;
    virtual void broadcastDeltaTimes(const std::vector<unsigned long> &deltaTimes) = 0;
    virtual void broadcastExtendedMetrics(Configurations::precision avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, Configurations::precision dragCoefficient) = 0;
    virtual bool isExtendedMetricsSubscribed() const = 0;
};
