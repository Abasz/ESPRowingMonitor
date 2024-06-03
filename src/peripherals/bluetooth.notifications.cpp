#include <numeric>
#include <string>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../utils/configuration.h"
#include "./bluetooth.service.h"

using std::array;
using std::to_string;

void BluetoothService::notifyBattery(const unsigned char batteryLevel) const
{
    batteryLevelCharacteristic->setValue(batteryLevel);
    if (batteryLevelCharacteristic->getSubscribedCount() > 0)
    {
        batteryLevelCharacteristic->notify();
    }
}

void BluetoothService::notifyBaseMetrics(const unsigned short revTime, const unsigned int revCount, const unsigned short strokeTime, const unsigned short strokeCount, const short avgStrokePower)
{
    if (baseMetricsParameters.characteristic->getSubscribedCount() == 0)
    {
        return;
    }

    baseMetricsParameters.revTime = revTime;
    baseMetricsParameters.revCount = revCount;
    baseMetricsParameters.strokeTime = strokeTime;
    baseMetricsParameters.strokeCount = strokeCount;
    baseMetricsParameters.avgStrokePower = avgStrokePower;

    const auto coreStackSize = 2'048U;

    if (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService)
    {
        xTaskCreatePinnedToCore(
            BaseMetricsParameters::pscTask,
            "notifyClients",
            coreStackSize,
            &baseMetricsParameters,
            1,
            NULL,
            0);

        return;
    }

    if (eepromService.getBleServiceFlag() == BleServiceFlag::CscService)
    {
        xTaskCreatePinnedToCore(
            BaseMetricsParameters::cscTask,
            "notifyClients",
            coreStackSize,
            &baseMetricsParameters,
            1,
            NULL,
            0);

        return;
    }
}

void BluetoothService::notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor)
{
    if (extendedMetricsParameters.characteristic->getSubscribedCount() == 0)
    {
        return;
    }

    const auto secInMicroSec = 1e6L;
    extendedMetricsParameters.avgStrokePower = avgStrokePower;
    extendedMetricsParameters.recoveryDuration = lround(recoveryDuration / secInMicroSec * 4'096);
    extendedMetricsParameters.driveDuration = lround(driveDuration / secInMicroSec * 4'096);
    extendedMetricsParameters.dragFactor = dragFactor;

    const auto coreStackSize = 1'800U;

    xTaskCreatePinnedToCore(
        ExtendedMetricParameters::task,
        "notifyExtendedMetrics",
        coreStackSize,
        &extendedMetricsParameters,
        1,
        NULL,
        0);
}

void BluetoothService::notifyHandleForces(const std::vector<float> &handleForces)
{
    if (handleForcesParameters.characteristic->getSubscribedCount() == 0 || handleForces.empty())
    {
        return;
    }

    handleForcesParameters.mtu = std::accumulate(handleForcesParameters.clientIds.cbegin(), handleForcesParameters.clientIds.cend(), 512, [&](unsigned short previousValue, unsigned short currentValue)
                                                 {  
                    const auto currentMTU = handleForcesParameters.characteristic->getService()->getServer()->getPeerMTU(currentValue);
                    if (currentMTU == 0)
                    {
                        return previousValue;
                    }

                return std::min(previousValue, currentMTU); });
    handleForcesParameters.handleForces = handleForces;

    const auto coreStackSize = 2'048U;
    const auto variableStackSize = handleForcesParameters.mtu > handleForcesParameters.handleForces.size() * sizeof(float) ? handleForcesParameters.handleForces.size() * sizeof(float) : handleForcesParameters.mtu;

    xTaskCreatePinnedToCore(
        HandleForcesParameters::task,
        "notifyHandleForces",
        coreStackSize + variableStackSize / 3,
        &handleForcesParameters,
        1,
        NULL,
        0);
}

void BluetoothService::notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes)
{
    if (deltaTimesParameters.characteristic->getSubscribedCount() == 0 || deltaTimes.empty())
    {
        return;
    }

    deltaTimesParameters.deltaTimes = deltaTimes;

    const auto coreStackSize = 1'850U;

    xTaskCreatePinnedToCore(
        DeltaTimesParameters::task,
        "notifyDeltaTimes",
        coreStackSize + deltaTimesParameters.deltaTimes.size() * sizeof(unsigned long) / 3,
        &deltaTimesParameters,
        1,
        NULL,
        0);
}

void BluetoothService::HandleForcesParameters::task(void *parameters)
{
    {
        const auto *const params = static_cast<const BluetoothService::HandleForcesParameters *>(parameters);

        const unsigned char chunkSize = (params->mtu - 3 - 2) / sizeof(float);
        const unsigned char split = params->handleForces.size() / chunkSize + (params->handleForces.size() % chunkSize == 0 ? 0 : 1);

        auto i = 0U;
        Log.verboseln("MTU of extended: %d, chunk size(bytes): %d, number of chunks: %d", params->mtu, chunkSize, split);

        while (i < split)
        {
            const auto end = (i + 1U) * chunkSize < params->handleForces.size() ? chunkSize * sizeof(float) : (params->handleForces.size() - i * chunkSize) * sizeof(float);
            std::vector<unsigned char> temp(end + 2);

            temp[0] = split;
            temp[1] = i + 1;
            memcpy(temp.data() + 2, params->handleForces.data() + i * chunkSize, end);

            params->characteristic->setValue(temp.data(), temp.size());
            params->characteristic->notify();
            i++;
        }
    }
    vTaskDelete(nullptr);
}

void BluetoothService::DeltaTimesParameters::task(void *parameters)
{
    {
        const auto *const params = static_cast<const BluetoothService::DeltaTimesParameters *>(parameters);

        params->characteristic->setValue((const unsigned char *)params->deltaTimes.data(), params->deltaTimes.size() * sizeof(unsigned long));
        params->characteristic->notify();
    }
    vTaskDelete(nullptr);
}

void BluetoothService::ExtendedMetricParameters::task(void *parameters)
{
    {
        const auto *const params = static_cast<const BluetoothService::ExtendedMetricParameters *>(parameters);

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

void BluetoothService ::BaseMetricsParameters::cscTask(void *parameters)
{
    {
        const auto *const params = static_cast<const BluetoothService::BaseMetricsParameters *>(parameters);
        const auto length = 11U;
        array<unsigned char, length> temp = {
            CSCSensorBleFlags::cscMeasurementFeaturesFlag,

            static_cast<unsigned char>(params->revCount),
            static_cast<unsigned char>(params->revCount >> 8),
            static_cast<unsigned char>(params->revCount >> 16),
            static_cast<unsigned char>(params->revCount >> 24),

            static_cast<unsigned char>(params->revTime),
            static_cast<unsigned char>(params->revTime >> 8),

            static_cast<unsigned char>(params->strokeCount),
            static_cast<unsigned char>(params->strokeCount >> 8),
            static_cast<unsigned char>(params->strokeTime),
            static_cast<unsigned char>(params->strokeTime >> 8)};
        params->characteristic->setValue(temp);
        params->characteristic->notify();
    }
    vTaskDelete(NULL);
}

void BluetoothService::BaseMetricsParameters ::pscTask(void *parameters)
{
    {
        const auto *const params = static_cast<const BluetoothService::BaseMetricsParameters *>(parameters);

        const auto length = 14U;
        array<unsigned char, length> temp = {
            static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag),
            static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag >> 8),

            static_cast<unsigned char>(params->avgStrokePower),
            static_cast<unsigned char>(params->avgStrokePower >> 8),

            static_cast<unsigned char>(params->revCount),
            static_cast<unsigned char>(params->revCount >> 8),
            static_cast<unsigned char>(params->revCount >> 16),
            static_cast<unsigned char>(params->revCount >> 24),
            static_cast<unsigned char>(params->revTime),
            static_cast<unsigned char>(params->revTime >> 8),

            static_cast<unsigned char>(params->strokeCount),
            static_cast<unsigned char>(params->strokeCount >> 8),
            static_cast<unsigned char>(params->strokeTime),
            static_cast<unsigned char>(params->strokeTime >> 8),
        };

        params->characteristic->setValue(temp);
        params->characteristic->notify();
    }
    vTaskDelete(NULL);
}

void BluetoothService::notifySettings() const
{
    settingsCharacteristic->setValue(getSettings());
    if (settingsCharacteristic->getSubscribedCount() > 0)
    {
        settingsCharacteristic->notify();
    }
}