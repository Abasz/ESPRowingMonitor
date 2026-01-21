#include <array>
#include <cstring>
#include <numeric>
#include <ranges>
#include <span>
#include <vector>

#include "ArduinoLog.h"

#include "../ble-metrics.model.h"
#include "../ble.enums.h"
#include "./extended-metrics.service.h"

using std::vector;

void ExtendedMetricBleService::broadcastHandleForces(const vector<float> &handleForces)
{
    ASSERT_SETUP_CALLED(handleForcesParams.characteristic);

    const unsigned short mtu = calculateMtu(handleForcesParams.callbacks.getClientIds());

    handleForcesParams.chunkSize = (mtu - 3U - 2U) / sizeof(float);
    handleForcesParams.handleForces = handleForces;

    const auto coreStackSize = 2'240U;
    const auto variableStackSize = mtu > handleForcesParams.handleForces.size() * sizeof(float) ? handleForcesParams.handleForces.size() * sizeof(float) : mtu;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::HandleForcesParams::task,
        "notifyHandleForces",
        coreStackSize + variableStackSize / 3,
        &handleForcesParams,
        1,
        nullptr,
        0);
}

void ExtendedMetricBleService::broadcastDeltaTimes(const vector<unsigned long> &deltaTimes)
{
    ASSERT_SETUP_CALLED(deltaTimesParams.characteristic);

    deltaTimesParams.deltaTimes = deltaTimes;

    const auto coreStackSize = 2'368U;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::DeltaTimesParams::task,
        "notifyDeltaTimes",
        coreStackSize + deltaTimesParams.deltaTimes.size() * sizeof(unsigned long) / 3,
        &deltaTimesParams,
        1,
        nullptr,
        0);
}

void ExtendedMetricBleService::broadcastExtendedMetrics(const Configurations::precision avgStrokePower, const unsigned int recoveryDuration, const unsigned int driveDuration, const Configurations::precision dragCoefficient)
{
    ASSERT_SETUP_CALLED(extendedMetricsParams.characteristic);

    extendedMetricsParams.avgStrokePower = avgStrokePower;
    extendedMetricsParams.recoveryDuration = recoveryDuration;
    extendedMetricsParams.driveDuration = driveDuration;
    extendedMetricsParams.dragCoefficient = dragCoefficient;

    const auto coreStackSize = 2'368U;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::ExtendedMetricsParams::task,
        "notifyExtendedMetrics",
        coreStackSize,
        &extendedMetricsParams,
        1,
        nullptr,
        0);
}

void ExtendedMetricBleService::ExtendedMetricsParams::task(void *parameters)
{
    {
        const auto *const params = static_cast<const ExtendedMetricBleService::ExtendedMetricsParams *>(parameters);

        const auto secInMicroSec = 1e6;
        const auto avgStrokePower = static_cast<short>(std::lround(params->avgStrokePower));
        const auto recoveryDuration = static_cast<unsigned short>(std::lround(params->recoveryDuration / secInMicroSec * 4'096));
        const auto driveDuration = static_cast<unsigned short>(std::lround(params->driveDuration / secInMicroSec * 4'096));
        const auto dragFactor = static_cast<unsigned short>(std::lround(params->dragCoefficient * 1e6));

        const auto length = 8U;
        std::array<unsigned char, length> temp = {
            static_cast<unsigned char>(avgStrokePower),
            static_cast<unsigned char>(avgStrokePower >> 8),

            static_cast<unsigned char>(driveDuration),
            static_cast<unsigned char>(driveDuration >> 8),
            static_cast<unsigned char>(recoveryDuration),
            static_cast<unsigned char>(recoveryDuration >> 8),

            static_cast<unsigned char>(dragFactor),
            static_cast<unsigned char>(dragFactor >> 8),
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

        const std::span<const float> handleForces(params->handleForces);
        const std::span<const std::byte> byteView = std::as_bytes(handleForces);

        const size_t chunkSizeInBytes = params->chunkSize * sizeof(float);
        const size_t totalBytes = byteView.size_bytes();
        const size_t split = (totalBytes + chunkSizeInBytes - 1) / chunkSizeInBytes;

        Log.verboseln("Chunk size(bytes): %u, number of chunks: %u", chunkSizeInBytes, split);

        std::vector<std::byte> buffer;
        buffer.reserve(chunkSizeInBytes + 2);

        size_t chunkIndex = 1;
        for (const auto &chunk : byteView | std::views::chunk(chunkSizeInBytes))
        {
            buffer.clear();

            buffer.push_back(static_cast<std::byte>(split));
            buffer.push_back(static_cast<std::byte>(chunkIndex++));

            buffer.insert(cend(buffer), cbegin(chunk), cend(chunk));

            params->characteristic->setValue(buffer);
            params->characteristic->notify();
        }
    }
    vTaskDelete(nullptr);
}

void ExtendedMetricBleService::DeltaTimesParams::task(void *parameters)
{
    {
        const auto *const params = static_cast<const ExtendedMetricBleService::DeltaTimesParams *>(parameters);

        const std::span<const unsigned long> deltaTimes{params->deltaTimes};
        const auto bytes = std::as_bytes(deltaTimes);
        params->characteristic->setValue(bytes);
        params->characteristic->notify();
    }
    vTaskDelete(nullptr);
}