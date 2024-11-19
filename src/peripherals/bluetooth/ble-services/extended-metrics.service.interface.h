#pragma once

#include <vector>

#include "NimBLEDevice.h"

using std::vector;

class IExtendedMetricBleService
{
protected:
    ~IExtendedMetricBleService() = default;

public:
    virtual NimBLEService *setup(NimBLEServer *server) = 0;

    virtual const vector<unsigned char> &getHandleForcesClientIds() const = 0;
    virtual void addHandleForcesClientId(unsigned char clientId) = 0;
    virtual const vector<unsigned char> &getDeltaTimesClientIds() const = 0;
    virtual void addDeltaTimesClientId(unsigned char clientId) = 0;

    virtual unsigned short getDeltaTimesClientMtu(unsigned char clientId) const = 0;

    virtual void broadcastHandleForces(const std::vector<float> &handleForces) = 0;
    virtual void broadcastDeltaTimes(const std::vector<unsigned long> &deltaTimes) = 0;
    virtual void broadcastExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor) = 0;

    virtual unsigned char removeDeltaTimesClient(unsigned char clientId) = 0;
    virtual unsigned char removeHandleForcesClient(unsigned char clientId) = 0;

    virtual bool isExtendedMetricsSubscribed() const = 0;
};
