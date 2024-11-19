#include <array>
#include <numeric>
#include <vector>

#include "esp_err.h"

#include "ArduinoLog.h"

#include "../../../utils/enums.h"
#include "./extended-metrics.service.h"

using std::vector;

void ExtendedMetricBleService::broadcastHandleForces(const vector<float> &handleForces)
{
    if (handleForcesParams.characteristic == nullptr)
    {
        Log.errorln("Extended metrics ble service has not been setup, restarting");

        ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);

        return;
    }

    const unsigned short mtu = std::accumulate(cbegin(handleForcesParams.clientIds), cend(handleForcesParams.clientIds), 512, [&](unsigned short previousMTU, unsigned short clientId)
                                               {
        const auto currentMTU = getClientHandleForcesMtu(clientId);
        if (currentMTU == 0)
        {
            return previousMTU;
        }

        return std::min(previousMTU, currentMTU); });

    handleForcesParams.chunkSize = calculateHandleForcesChunkSize(mtu);
    handleForcesParams.handleForces = handleForces;

    const auto coreStackSize = 2'048U;
    const auto variableStackSize = mtu > handleForcesParams.handleForces.size() * sizeof(float) ? handleForcesParams.handleForces.size() * sizeof(float) : mtu;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::HandleForcesParams::task,
        "notifyHandleForces",
        coreStackSize + variableStackSize / 3,
        &handleForcesParams,
        1,
        NULL,
        0);
}

void ExtendedMetricBleService::broadcastDeltaTimes(const vector<unsigned long> &deltaTimes)
{
    if (deltaTimesParams.characteristic == nullptr)
    {
        Log.errorln("Extended metrics ble service has not been setup, restarting");

        ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);

        return;
    }

    deltaTimesParams.deltaTimes = deltaTimes;

    const auto coreStackSize = 1'850U;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::DeltaTimesParams::task,
        "notifyDeltaTimes",
        coreStackSize + deltaTimesParams.deltaTimes.size() * sizeof(unsigned long) / 3,
        &deltaTimesParams,
        1,
        NULL,
        0);
}

void ExtendedMetricBleService::broadcastExtendedMetrics(const short avgStrokePower, const unsigned int recoveryDuration, const unsigned int driveDuration, const unsigned char dragFactor)
{
    if (extendedMetricsParams.characteristic == nullptr)
    {
        Log.errorln("Extended metrics ble service has not been setup, restarting");

        ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);

        return;
    }

    const auto secInMicroSec = 1e6;
    extendedMetricsParams.avgStrokePower = avgStrokePower;
    extendedMetricsParams.recoveryDuration = lround(recoveryDuration / secInMicroSec * 4'096);
    extendedMetricsParams.driveDuration = lround(driveDuration / secInMicroSec * 4'096);
    extendedMetricsParams.dragFactor = dragFactor;

    const auto coreStackSize = 1'800U;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::ExtendedMetricsParams::task,
        "notifyExtendedMetrics",
        coreStackSize,
        &extendedMetricsParams,
        1,
        NULL,
        0);
}

void ExtendedMetricBleService::ExtendedMetricsParams::task(void *parameters)
{
    {
        const auto *const params = static_cast<const ExtendedMetricBleService::ExtendedMetricsParams *>(parameters);

        const auto length = 7U;
        std::array<unsigned char, length> temp = {
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

        const unsigned char split = params->handleForces.size() / params->chunkSize + (params->handleForces.size() % params->chunkSize == 0 ? 0 : 1);

        auto i = 0UL;
        Log.verboseln("Chunk size(bytes): %d, number of chunks: %d", params->chunkSize, split);

        while (i < split)
        {
            const auto end = (i + 1U) * params->chunkSize < params->handleForces.size() ? params->chunkSize * sizeof(float) : (params->handleForces.size() - i * params->chunkSize) * sizeof(float);
            vector<unsigned char> temp(end + 2);

            temp[0] = split;
            temp[1] = i + 1;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            memcpy(temp.data() + 2, params->handleForces.data() + i * params->chunkSize, end);

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