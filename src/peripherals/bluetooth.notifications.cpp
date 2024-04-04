#include <numeric>
#include <string>

#include "NimBLEDevice.h"

#include "../utils/configuration.h"
#include "bluetooth.service.h"

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

void BluetoothService::notifyDragFactor(const unsigned short distance, const unsigned char dragFactor) const
{
    if constexpr (!Configurations::hasExtendedBleMetrics)
    {
        std::string value = "DF=" + to_string(dragFactor) + ", Dist=" + to_string(distance);
        dragFactorCharacteristic->setValue(value);
        if (dragFactorCharacteristic->getSubscribedCount() > 0)
        {
            dragFactorCharacteristic->notify();
        }
    }
}

void BluetoothService::notifyClients(const unsigned short revTime, const unsigned int revCount, const unsigned short strokeTime, const unsigned short strokeCount, const short avgStrokePower) const
{
    if constexpr (Configurations::isBleServiceEnabled)
    {
        if (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService)
        {
            notifyPsc(revTime, revCount, strokeTime, strokeCount, avgStrokePower);
        }
        if (eepromService.getBleServiceFlag() == BleServiceFlag::CscService)
        {
            notifyCsc(revTime, revCount, strokeTime, strokeCount);
        }
    }
}

void BluetoothService::notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor) const
{
    if (extendedMetricsCharacteristic->getSubscribedCount() == 0)
    {
        return;
    }

    const auto secInMicroSec = 1e6L;
    const unsigned short recoveryDurationBle = lround(recoveryDuration / secInMicroSec * 4'096);
    const unsigned short driveDurationBle = lround(driveDuration / secInMicroSec * 4'096);

    const auto length = 7U;
    array<unsigned char, length> temp = {
        static_cast<unsigned char>(avgStrokePower),
        static_cast<unsigned char>(avgStrokePower >> 8),

        static_cast<unsigned char>(driveDurationBle),
        static_cast<unsigned char>(driveDurationBle >> 8),
        static_cast<unsigned char>(recoveryDurationBle),
        static_cast<unsigned char>(recoveryDurationBle >> 8),

        dragFactor,
    };

    extendedMetricsCharacteristic->setValue(temp);
    extendedMetricsCharacteristic->notify();
}

void BluetoothService::notifyHandleForces(const std::vector<float> &handleForces) const
{
    if (handleForcesCharacteristic->getSubscribedCount() == 0 || handleForces.empty())
    {
        return;
    }

    const auto mtu = std::accumulate(handleForcesClientIds.cbegin(), handleForcesClientIds.cend(), 512, [&](unsigned short previousValue, unsigned short currentValue)
                                     {  
                    const auto currentMTU = handleForcesCharacteristic->getService()->getServer()->getPeerMTU(currentValue);
                    if (currentMTU == 0)
                    {
                        return previousValue;
                    }

                return std::min(previousValue, currentMTU); });

    const unsigned char chunkSize = (mtu - 3 - 2) / sizeof(float);
    const unsigned char split = handleForces.size() / chunkSize + (handleForces.size() % chunkSize == 0 ? 0 : 1);

    auto i = 0U;
    Log.traceln("MTU of extended: %d, chunk size(bytes): %d, number of chunks: %d", mtu, chunkSize, split);

    while (i < split)
    {
        const auto end = (i + 1U) * chunkSize < handleForces.size() ? chunkSize * sizeof(float) : (handleForces.size() - i * chunkSize) * sizeof(float);
        std::vector<unsigned char> temp(end + 2);

        temp[0] = split;
        temp[1] = i + 1;
        memcpy(temp.data() + 2, handleForces.data() + i * chunkSize, end);

        handleForcesCharacteristic->setValue(temp.data(), temp.size());
        handleForcesCharacteristic->notify();
        delay(1);
        i++;
    }
}

void BluetoothService::notifyCsc(const unsigned short revTime, const unsigned int revCount, const unsigned short strokeTime, const unsigned short strokeCount) const
{
    if (cscMeasurementCharacteristic->getSubscribedCount() == 0)
    {
        return;
    }

    const auto length = 11U;
    array<unsigned char, length> temp = {
        CSCSensorBleFlags::cscMeasurementFeaturesFlag,

        static_cast<unsigned char>(revCount),
        static_cast<unsigned char>(revCount >> 8),
        static_cast<unsigned char>(revCount >> 16),
        static_cast<unsigned char>(revCount >> 24),

        static_cast<unsigned char>(revTime),
        static_cast<unsigned char>(revTime >> 8),

        static_cast<unsigned char>(strokeCount),
        static_cast<unsigned char>(strokeCount >> 8),
        static_cast<unsigned char>(strokeTime),
        static_cast<unsigned char>(strokeTime >> 8)};

    cscMeasurementCharacteristic->setValue(temp);
    cscMeasurementCharacteristic->notify();
}

void BluetoothService::notifyPsc(const unsigned short revTime, const unsigned int revCount, const unsigned short strokeTime, const unsigned short strokeCount, const short avgStrokePower) const
{
    if (pscMeasurementCharacteristic->getSubscribedCount() == 0)
    {
        return;
    }

    const auto length = 14U;
    array<unsigned char, length> temp = {
        static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag),
        static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag >> 8),

        static_cast<unsigned char>(avgStrokePower),
        static_cast<unsigned char>(avgStrokePower >> 8),

        static_cast<unsigned char>(revCount),
        static_cast<unsigned char>(revCount >> 8),
        static_cast<unsigned char>(revCount >> 16),
        static_cast<unsigned char>(revCount >> 24),
        static_cast<unsigned char>(revTime),
        static_cast<unsigned char>(revTime >> 8),

        static_cast<unsigned char>(strokeCount),
        static_cast<unsigned char>(strokeCount >> 8),
        static_cast<unsigned char>(strokeTime),
        static_cast<unsigned char>(strokeTime >> 8),
    };

    pscMeasurementCharacteristic->setValue(temp);
    pscMeasurementCharacteristic->notify();
}

void BluetoothService::notifySettings() const
{
    settingsCharacteristic->setValue(getSettings());
    if (settingsCharacteristic->getSubscribedCount() > 0)
    {
        settingsCharacteristic->notify();
    }
}