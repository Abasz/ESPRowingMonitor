#include <numeric>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../utils/configuration.h"
#include "./ble-services/base-metrics.service.interface.h"
#include "./bluetooth.controller.h"

void BluetoothController::notifyBattery(const unsigned char batteryLevel) const
{
    batteryBleService.setBatteryLevel(batteryLevel);
    if (batteryBleService.isSubscribed())
    {
        batteryBleService.broadcastBatteryLevel();
    }
}

void BluetoothController::notifyBaseMetrics(const unsigned short revTime, const unsigned int revCount, const unsigned short strokeTime, const unsigned short strokeCount, const short avgStrokePower)
{
    if (!baseMetricsBleService.isSubscribed())
    {
        return;
    }

    baseMetricsBleService.broadcastBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);
}

void BluetoothController::notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor)
{
    if (extendedMetricsBleService.extendedMetricsParams.characteristic->getSubscribedCount() == 0)
    {
        return;
    }

    const auto secInMicroSec = 1e6;
    extendedMetricsBleService.extendedMetricsParams.avgStrokePower = avgStrokePower;
    extendedMetricsBleService.extendedMetricsParams.recoveryDuration = lround(recoveryDuration / secInMicroSec * 4'096);
    extendedMetricsBleService.extendedMetricsParams.driveDuration = lround(driveDuration / secInMicroSec * 4'096);
    extendedMetricsBleService.extendedMetricsParams.dragFactor = dragFactor;

    const auto coreStackSize = 1'800U;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::ExtendedMetricsParams::task,
        "notifyExtendedMetrics",
        coreStackSize,
        &extendedMetricsBleService.extendedMetricsParams,
        1,
        NULL,
        0);
}

void BluetoothController::notifyHandleForces(const std::vector<float> &handleForces)
{
    const auto clientIds = extendedMetricsBleService.getHandleForcesClientIds();

    if (clientIds.empty() || handleForces.empty())
    {
        return;
    }

    extendedMetricsBleService.handleForcesParams.mtu = std::accumulate(cbegin(clientIds), cend(clientIds), 512, [&](unsigned short previousValue, unsigned short currentValue)
                                                                       {  
                    const auto currentMTU = extendedMetricsBleService.getHandleForcesMTU(currentValue);
                    if (currentMTU == 0)
                    {
                        return previousValue;
                    }

                return std::min(previousValue, currentMTU); });
    extendedMetricsBleService.handleForcesParams.handleForces = handleForces;

    const auto coreStackSize = 2'048U;
    const auto variableStackSize = extendedMetricsBleService.handleForcesParams.mtu > extendedMetricsBleService.handleForcesParams.handleForces.size() * sizeof(float) ? extendedMetricsBleService.handleForcesParams.handleForces.size() * sizeof(float) : extendedMetricsBleService.handleForcesParams.mtu;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::HandleForcesParams::task,
        "notifyHandleForces",
        coreStackSize + variableStackSize / 3,
        &extendedMetricsBleService.handleForcesParams,
        1,
        NULL,
        0);
}

void BluetoothController::notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes)
{
    if (extendedMetricsBleService.deltaTimesParams.characteristic->getSubscribedCount() == 0 || deltaTimes.empty())
    {
        return;
    }

    extendedMetricsBleService.deltaTimesParams.deltaTimes = deltaTimes;

    const auto coreStackSize = 1'850U;

    xTaskCreatePinnedToCore(
        ExtendedMetricBleService::DeltaTimesParams::task,
        "notifyDeltaTimes",
        coreStackSize + extendedMetricsBleService.deltaTimesParams.deltaTimes.size() * sizeof(unsigned long) / 3,
        &extendedMetricsBleService.deltaTimesParams,
        1,
        NULL,
        0);
}
