#include <array>
#include <numeric>
#include <string>
#include <utility>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../utils/configuration.h"
#include "./bluetooth.controller.h"

BluetoothController::BluetoothController(IEEPROMService &_eepromService, IOtaUpdaterService &_otaService, ISettingsBleService &_settingsBleService, IBatteryBleService &_batteryBleService, IDeviceInfoBleService &_deviceInfoBleService, IOtaBleService &_otaBleService, IBaseMetricsBleService &_baseMetricsBleService, IExtendedMetricBleService &_extendedMetricsBleService) : eepromService(_eepromService), otaService(_otaService), settingsBleService(_settingsBleService), batteryBleService(_batteryBleService), deviceInfoBleService(_deviceInfoBleService), otaBleService(_otaBleService), baseMetricsBleService(_baseMetricsBleService), extendedMetricsBleService(_extendedMetricsBleService), serverCallbacks(_extendedMetricsBleService)
{
    if constexpr (Configurations::enableBluetoothDeltaTimeLogging)
    {
        const auto maxMtu = 512 / sizeof(unsigned long);
        bleDeltaTimes.reserve(maxMtu);
    }
}

void BluetoothController::update()
{
    const auto now = millis();
    const unsigned int bleUpdateInterval = 1'000;
    if (now - lastMetricsBroadcastTime > bleUpdateInterval && baseMetricsBleService.isSubscribed())
    {
        baseMetricsBleService.broadcastBaseMetrics(bleData);
        lastMetricsBroadcastTime = now;
    }

    if constexpr (Configurations::enableBluetoothDeltaTimeLogging)
    {
        const auto &clientIds = extendedMetricsBleService.getDeltaTimesClientIds();
        if (now - lastDeltaTimesBroadcastTime > bleUpdateInterval && !bleDeltaTimes.empty() && !clientIds.empty())
        {
            flushBleDeltaTimes(extendedMetricsBleService.calculateMtu(clientIds));
        }
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

void BluetoothController::notifyBattery(const unsigned char batteryLevel) const
{
    batteryBleService.setBatteryLevel(batteryLevel);
    if (batteryBleService.isSubscribed())
    {
        batteryBleService.broadcastBatteryLevel();
    }
}

void BluetoothController::notifyNewDeltaTime(unsigned long deltaTime)
{
    const auto &clientIds = extendedMetricsBleService.getDeltaTimesClientIds();
    if (clientIds.empty())
    {
        return;
    }

    const auto mtu = extendedMetricsBleService.calculateMtu(clientIds);

    const auto minimumMtu = 100U;
    if (mtu < minimumMtu)
    {
        return;
    }

    bleDeltaTimes.push_back(deltaTime);

    if ((bleDeltaTimes.size() + 1U) * sizeof(unsigned long) > mtu - 3U)
    {
        flushBleDeltaTimes(mtu);
    }
}

void BluetoothController::notifyNewMetrics(const RowingDataModels::RowingMetrics &data)
{
    bleData = {
        .revTime = data.lastRevTime,
        .distance = data.distance,
        .strokeTime = data.lastStrokeTime,
        .strokeCount = data.strokeCount,
        .avgStrokePower = data.avgStrokePower,
    };

    if constexpr (Configurations::hasExtendedBleMetrics)
    {
        const auto isSubscribed = !extendedMetricsBleService.getHandleForcesClientIds().empty();
        if (isSubscribed && !data.driveHandleForces.empty())
        {
            extendedMetricsBleService.broadcastHandleForces(data.driveHandleForces);
        }

        if (extendedMetricsBleService.isExtendedMetricsSubscribed())
        {
            extendedMetricsBleService.broadcastExtendedMetrics(data.avgStrokePower, data.recoveryDuration, data.driveDuration, data.dragCoefficient);
        }
    }

    if (baseMetricsBleService.isSubscribed())
    {
        baseMetricsBleService.broadcastBaseMetrics(bleData);
    }

    lastMetricsBroadcastTime = millis();
}

void BluetoothController::flushBleDeltaTimes(const unsigned short mtu = 512U)
{
    extendedMetricsBleService.broadcastDeltaTimes(bleDeltaTimes);

    vector<unsigned long> clear;
    clear.reserve(mtu / sizeof(unsigned long) + 1U);
    bleDeltaTimes.swap(clear);

    lastDeltaTimesBroadcastTime = millis();
}