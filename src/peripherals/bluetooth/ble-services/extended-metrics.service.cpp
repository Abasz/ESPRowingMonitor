#include <numeric>
#include <vector>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../../utils/enums.h"
#include "./extended-metrics.service.h"

using std::vector;

ExtendedMetricBleService::ExtendedMetricBleService()
{
}

NimBLEService *ExtendedMetricBleService::setup(NimBLEServer *const server)
{
    Log.infoln("Setting up Extended Metrics Services");
    auto *extendedMetricsService = server->createService(CommonBleFlags::extendedMetricsServiceUuid);

    handleForcesParams.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::handleForcesUuid, NIMBLE_PROPERTY::NOTIFY);
    handleForcesParams.characteristic->setCallbacks(&handleForcesParams.callbacks);
    deltaTimesParams.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::deltaTimesUuid, NIMBLE_PROPERTY::NOTIFY);
    deltaTimesParams.characteristic->setCallbacks(&deltaTimesParams.callbacks);

    extendedMetricsParams.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::extendedMetricsUuid, NIMBLE_PROPERTY::NOTIFY);
    extendedMetricsParams.characteristic->setCallbacks(&extendedMetricsParams.callbacks);

    return extendedMetricsService;
}

const vector<unsigned char> &ExtendedMetricBleService::getHandleForcesClientIds() const
{
    return handleForcesParams.callbacks.getClientIds();
}

const vector<unsigned char> &ExtendedMetricBleService::getDeltaTimesClientIds() const
{
    return deltaTimesParams.callbacks.getClientIds();
}

const vector<unsigned char> &ExtendedMetricBleService::getExtendedMetricsClientIds() const
{
    return extendedMetricsParams.callbacks.getClientIds();
}

unsigned short ExtendedMetricBleService::calculateMtu(const std::vector<unsigned char> &clientIds) const
{
    auto *server = NimBLEDevice::getServer();

    return std::accumulate(cbegin(clientIds), cend(clientIds), 512, [&](unsigned short previousMtu, unsigned short clientId)
                           {
                    const auto currentMTU = server->getPeerMTU(clientId);
                    if (currentMTU == 0)
                    {
                        return previousMtu;
                    }

                    return std::min(previousMtu, currentMTU); });
}