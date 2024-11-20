#include <numeric>
#include <vector>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../../utils/enums.h"
#include "./extended-metrics.service.h"

using std::vector;

ExtendedMetricBleService::ExtendedMetricBleService() : callbacks(*this)
{
}

NimBLEService *ExtendedMetricBleService::setup(NimBLEServer *const server)
{
    Log.infoln("Setting up Extended Metrics Services");
    auto *extendedMetricsService = server->createService(CommonBleFlags::extendedMetricsServiceUuid);

    handleForcesParams.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::handleForcesUuid, NIMBLE_PROPERTY::NOTIFY);
    handleForcesParams.characteristic->setCallbacks(&callbacks);
    deltaTimesParams.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::deltaTimesUuid, NIMBLE_PROPERTY::NOTIFY);
    deltaTimesParams.characteristic->setCallbacks(&callbacks);

    extendedMetricsParams.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::extendedMetricsUuid, NIMBLE_PROPERTY::NOTIFY);

    return extendedMetricsService;
}

const vector<unsigned char> &ExtendedMetricBleService::getHandleForcesClientIds() const
{
    return handleForcesParams.clientIds;
}

void ExtendedMetricBleService::addHandleForcesClientId(unsigned char clientId)
{
    handleForcesParams.clientIds.push_back(clientId);
}

const vector<unsigned char> &ExtendedMetricBleService::getDeltaTimesClientIds() const
{
    return deltaTimesParams.clientIds;
}

void ExtendedMetricBleService::addDeltaTimesClientId(const unsigned char clientId)
{
    deltaTimesParams.clientIds.push_back(clientId);
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

unsigned char ExtendedMetricBleService::removeDeltaTimesClient(const unsigned char clientId)
{
    const auto initialSize = deltaTimesParams.clientIds.size();

    deltaTimesParams.clientIds.erase(
        std::remove_if(
            begin(deltaTimesParams.clientIds),
            end(deltaTimesParams.clientIds),
            [&](unsigned char connectionId)
            {
                return connectionId == clientId;
            }),
        cend(deltaTimesParams.clientIds));

    return initialSize - deltaTimesParams.clientIds.size();
}

unsigned char ExtendedMetricBleService::removeHandleForcesClient(const unsigned char clientId)
{
    const auto initialSize = handleForcesParams.clientIds.size();
    handleForcesParams.clientIds.erase(
        std::remove_if(
            begin(handleForcesParams.clientIds),
            end(handleForcesParams.clientIds),
            [&](unsigned char connectionId)
            {
                return connectionId == clientId;
            }),
        cend(handleForcesParams.clientIds));

    return initialSize - handleForcesParams.clientIds.size();
}

bool ExtendedMetricBleService::isExtendedMetricsSubscribed() const
{
    if (extendedMetricsParams.characteristic == nullptr)
    {
        return false;
    }

    return extendedMetricsParams.characteristic->getSubscribedCount() > 0;
}