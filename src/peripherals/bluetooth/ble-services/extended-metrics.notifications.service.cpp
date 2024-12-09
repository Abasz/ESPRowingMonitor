#include <array>
#include <numeric>
#include <vector>

#include "ArduinoLog.h"

#include "../../../utils/enums.h"
#include "../ble-metrics.model.h"
#include "./extended-metrics.service.h"

using std::vector;

void ExtendedMetricBleService::broadcastHandleForces(const vector<float> &handleForces)
{
    ASSERT_SETUP_CALLED(handleForcesParams.characteristic);

    const unsigned short mtu = calculateMtu(handleForcesParams.clientIds);

    handleForcesParams.chunkSize = (mtu - 3U - 2U) / sizeof(float);
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
    ASSERT_SETUP_CALLED(deltaTimesParams.characteristic);

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

void ExtendedMetricBleService::broadcastExtendedMetrics(const Configurations::precision avgStrokePower, const unsigned int recoveryDuration, const unsigned int driveDuration, const Configurations::precision dragCoefficient)
{
    ASSERT_SETUP_CALLED(extendedMetricsParams.characteristic);

    extendedMetricsParams.avgStrokePower = avgStrokePower;
    extendedMetricsParams.recoveryDuration = recoveryDuration;
    extendedMetricsParams.driveDuration = driveDuration;
    extendedMetricsParams.dragCoefficient = dragCoefficient;

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

        const auto secInMicroSec = 1e6;
        const auto avgStrokePower = static_cast<short>(lround(params->avgStrokePower));
        const auto recoveryDuration = static_cast<unsigned short>(lround(params->recoveryDuration / secInMicroSec * 4'096));
        const auto driveDuration = static_cast<unsigned short>(lround(params->driveDuration / secInMicroSec * 4'096));
        const auto dragFactor = static_cast<unsigned char>(lround(params->dragCoefficient * 1e6));

        const auto length = 7U;
        std::array<unsigned char, length> temp = {
            static_cast<unsigned char>(avgStrokePower),
            static_cast<unsigned char>(avgStrokePower >> 8),

            static_cast<unsigned char>(driveDuration),
            static_cast<unsigned char>(driveDuration >> 8),
            static_cast<unsigned char>(recoveryDuration),
            static_cast<unsigned char>(recoveryDuration >> 8),

            dragFactor,
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
            ++i;
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