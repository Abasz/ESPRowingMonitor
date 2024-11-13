#include <array>

#include "ArduinoLog.h"

#include "../../../utils/enums.h"
#include "./extended-metrics.service.h"

using std::array;

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

unsigned short ExtendedMetricBleService::getDeltaTimesMTU(const unsigned char clientId) const
{
    return deltaTimesParams.characteristic->getService()->getServer()->getPeerMTU(clientId);
}

unsigned short ExtendedMetricBleService::getHandleForcesMTU(const unsigned char clientId) const
{
    return handleForcesParams.characteristic->getService()->getServer()->getPeerMTU(clientId);
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

void ExtendedMetricBleService::ExtendedMetricsParams::task(void *parameters)
{
    {
        const auto *const params = static_cast<const ExtendedMetricBleService::ExtendedMetricsParams *>(parameters);

        const auto length = 7U;
        array<unsigned char, length> temp = {
            static_cast<unsigned char>(params->avgStrokePower),
            static_cast<unsigned char>(params->avgStrokePower >> 8),

            static_cast<unsigned char>(params->driveDuration),
            static_cast<unsigned char>(params->driveDuration >> 8),
            static_cast<unsigned char>(params->recoveryDuration),
            static_cast<unsigned char>(params->recoveryDuration >> 8),

            params->dragFactor,
        };

        params->characteristic->setValue(temp);
        params->characteristic->notify();
    }
    vTaskDelete(nullptr);
}

void ExtendedMetricBleService::HandleForcesParams::task(void *parameters)
{
    {
        const auto *const params = static_cast<const ExtendedMetricBleService::HandleForcesParams *>(parameters);

        const unsigned char chunkSize = (params->mtu - 3 - 2) / sizeof(float);
        const unsigned char split = params->handleForces.size() / chunkSize + (params->handleForces.size() % chunkSize == 0 ? 0 : 1);

        auto i = 0UL;
        Log.verboseln("MTU of extended: %d, chunk size(bytes): %d, number of chunks: %d", params->mtu, chunkSize, split);

        while (i < split)
        {
            const auto end = (i + 1U) * chunkSize < params->handleForces.size() ? chunkSize * sizeof(float) : (params->handleForces.size() - i * chunkSize) * sizeof(float);
            std::vector<unsigned char> temp(end + 2);

            temp[0] = split;
            temp[1] = i + 1;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            memcpy(temp.data() + 2, params->handleForces.data() + i * chunkSize, end);

            params->characteristic->setValue(temp.data(), temp.size());
            params->characteristic->notify();
            i++;
        }
    }
    vTaskDelete(nullptr);
}

void ExtendedMetricBleService::DeltaTimesParams::task(void *parameters)
{
    {
        const auto *const params = static_cast<const ExtendedMetricBleService::DeltaTimesParams *>(parameters);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        params->characteristic->setValue((const unsigned char *)params->deltaTimes.data(), params->deltaTimes.size() * sizeof(unsigned long));
        params->characteristic->notify();
    }
    vTaskDelete(nullptr);
}