#include <array>

#include "ArduinoLog.h"

#include "../../../utils/enums.h"
#include "./extended-metrics.service.h"

using std::array;

ExtendedMetricBleService::ExtendedMetricBleService(BluetoothController &_bleController) : callbacks(_bleController)
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

void ExtendedMetricBleService::extendedMetricsTask(void *parameters)
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

void ExtendedMetricBleService::handleForcesTask(void *parameters)
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

void ExtendedMetricBleService::deltaTimesTask(void *parameters)
{
    {
        const auto *const params = static_cast<const ExtendedMetricBleService::DeltaTimesParams *>(parameters);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        params->characteristic->setValue((const unsigned char *)params->deltaTimes.data(), params->deltaTimes.size() * sizeof(unsigned long));
        params->characteristic->notify();
    }
    vTaskDelete(nullptr);
}