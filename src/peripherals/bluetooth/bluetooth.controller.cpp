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

void BluetoothController::update()
{
    const auto now = millis();
    const unsigned int bleUpdateInterval = 1'000;
    if (now - lastMetricsBroadcastTime > bleUpdateInterval && baseMetricsBleService.isSubscribed())
    {
        baseMetricsBleService.broadcastBaseMetrics(bleRevTimeData, bleRevCountData, bleStrokeTimeData, bleStrokeCountData, bleAvgStrokePowerData);
        lastMetricsBroadcastTime = now;
    }
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

    return extendedMetricsBleService.calculateMtu(clientIds);
}

void BluetoothController::notifyBattery(const unsigned char batteryLevel) const
{
    batteryBleService.setBatteryLevel(batteryLevel);
    if (batteryBleService.isSubscribed())
    {
        batteryBleService.broadcastBatteryLevel();
    }
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

void BluetoothController::notifyNewMetrics(const RowingDataModels::RowingMetrics &data)
{
    const auto secInMicroSec = 1e6L;
    bleRevTimeData = lroundl((data.lastRevTime / secInMicroSec) * (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService ? 2'048 : 1'024)) % USHRT_MAX;
    bleRevCountData = lround(data.distance);
    bleStrokeTimeData = lroundl((data.lastStrokeTime / secInMicroSec) * 1'024) % USHRT_MAX;
    bleStrokeCountData = data.strokeCount;
    bleAvgStrokePowerData = static_cast<short>(lround(data.avgStrokePower));

    if constexpr (Configurations::hasExtendedBleMetrics)
    {
        const auto isSubscribed = !extendedMetricsBleService.getHandleForcesClientIds().empty();
        if (isSubscribed && !data.driveHandleForces.empty())
        {
            extendedMetricsBleService.broadcastHandleForces(data.driveHandleForces);
        }

        if (extendedMetricsBleService.isExtendedMetricsSubscribed())
        {
            extendedMetricsBleService.broadcastExtendedMetrics(bleAvgStrokePowerData, data.recoveryDuration, data.driveDuration, lround(data.dragCoefficient * 1e6));
        }
    }

    if (baseMetricsBleService.isSubscribed())
    {
        baseMetricsBleService.broadcastBaseMetrics(bleRevTimeData, bleRevCountData, bleStrokeTimeData, bleStrokeCountData, bleAvgStrokePowerData);
    }

    lastMetricsBroadcastTime = millis();
}
