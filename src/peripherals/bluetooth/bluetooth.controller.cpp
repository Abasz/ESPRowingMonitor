#include <array>
#include <numeric>
#include <string>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../utils/configuration.h"
#include "./bluetooth.controller.h"

BluetoothController::BluetoothController(IEEPROMService &_eepromService, IOtaUpdaterService &_otaService, ISettingsBleService &_settingsBleService, IBatteryBleService &_batteryBleService, IDeviceInfoBleService &_deviceInfoBleService, IOtaBleService &_otaBleService, IBaseMetricsBleService &_baseMetricsBleService, IExtendedMetricBleService &_extendedMetricsBleService) : eepromService(_eepromService), otaService(_otaService), settingsBleService(_settingsBleService), batteryBleService(_batteryBleService), deviceInfoBleService(_deviceInfoBleService), otaBleService(_otaBleService), baseMetricsBleService(_baseMetricsBleService), extendedMetricsBleService(_extendedMetricsBleService), serverCallbacks(_extendedMetricsBleService)
{
}

bool BluetoothController::isAnyDeviceConnected()
{
    return NimBLEDevice::getServer()->getConnectedCount() > 0;
}

void BluetoothController::setup()
{
    setupBleDevice();
    startBLEServer();
}

void BluetoothController::startBLEServer()
{
    NimBLEDevice::getAdvertising()->start();
    Log.verboseln("Waiting a client connection to notify...");
}

void BluetoothController::stopServer()
{
    NimBLEDevice::getAdvertising()->stop();
}

void BluetoothController::setupBleDevice()
{
    Log.verboseln("Initializing BLE device");

    const auto deviceName = Configurations::deviceName + " (" + std::string(eepromService.getBleServiceFlag() == BleServiceFlag::CscService ? "CSC)" : "CPS)");
    NimBLEDevice::init(deviceName);
    NimBLEDevice::setPower(static_cast<esp_power_level_t>(Configurations::bleSignalStrength), ESP_BLE_PWR_TYPE_ADV);
    NimBLEDevice::setPower(static_cast<esp_power_level_t>(Configurations::bleSignalStrength), ESP_BLE_PWR_TYPE_DEFAULT);

    Log.verboseln("Setting up Server");

    auto *const pServer = NimBLEDevice::createServer();

    pServer->setCallbacks(&serverCallbacks);

    setupServices();
    setupAdvertisement();
}

void BluetoothController::setupServices()
{
    Log.verboseln("Setting up BLE Services");
    auto *server = NimBLEDevice::getServer();

    baseMetricsBleService.setup(server, eepromService.getBleServiceFlag())->start();

    if constexpr (Configurations::hasExtendedBleMetrics)
    {
        extendedMetricsBleService.setup(server)->start();
    }

    if constexpr (Configurations::batteryPinNumber != GPIO_NUM_NC)
    {
        batteryBleService.setup(server)->start();
    }

    settingsBleService.setup(server)->start();

    otaBleService.setup(server)->start();
    otaService.begin(otaBleService.getOtaTx());

    deviceInfoBleService.setup(server)->start();

    Log.verboseln("Starting BLE Server");

    server->start();
}

void BluetoothController::setupAdvertisement() const
{
    auto *pAdvertising = NimBLEDevice::getAdvertising();
    if (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService)
    {
        pAdvertising->setAppearance(PSCSensorBleFlags::bleAppearanceCyclingPower);
        pAdvertising->addServiceUUID(PSCSensorBleFlags::cyclingPowerSvcUuid);
    }
    if (eepromService.getBleServiceFlag() == BleServiceFlag::CscService)
    {
        pAdvertising->setAppearance(CSCSensorBleFlags::bleAppearanceCyclingSpeedCadence);
        pAdvertising->addServiceUUID(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid);
    }
}

unsigned short BluetoothController::calculateDeltaTimesMtu() const
{
    const auto clientIds = extendedMetricsBleService.getDeltaTimesClientIds();

    if (clientIds.empty())
    {
        return 0;
    }

    return std::accumulate(cbegin(clientIds), cend(clientIds), 512, [&](unsigned short previousMTU, unsigned short clientId)
                           {
                    const auto currentMTU = extendedMetricsBleService.getDeltaTimesClientMtu(clientId);
                    if (currentMTU == 0)
                    {
                        return previousMTU;
                    }

                    return std::min(previousMTU, currentMTU); });
}

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
    if (!extendedMetricsBleService.isExtendedMetricsSubscribed())
    {
        return;
    }

    extendedMetricsBleService.broadcastExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragFactor);
}

void BluetoothController::notifyHandleForces(const std::vector<float> &handleForces)
{
    const auto isSubscribed = !extendedMetricsBleService.getHandleForcesClientIds().empty();
    if (!isSubscribed || handleForces.empty())
    {
        return;
    }

    extendedMetricsBleService.broadcastHandleForces(handleForces);
}

void BluetoothController::notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes)
{
    const auto isSubscribed = !extendedMetricsBleService.getDeltaTimesClientIds().empty();
    if (!isSubscribed || deltaTimes.empty())
    {
        return;
    }

    extendedMetricsBleService.broadcastDeltaTimes(deltaTimes);
}
