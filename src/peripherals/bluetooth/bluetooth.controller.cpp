#include <array>
#include <numeric>
#include <string>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../../utils/configuration.h"
#include "./ble-services/device-info.service.h"
#include "./bluetooth.controller.h"

BluetoothController::BluetoothController(IEEPROMService &_eepromService, IOtaUploaderService &_otaService, ISettingsBleService &_settingsBleService) : eepromService(_eepromService), otaService(_otaService), settingsBleService(_settingsBleService), baseMetricsBleService(_settingsBleService, _eepromService), otaBleService(_otaService), serverCallbacks(extendedMetricsBleService)
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

    otaService.begin(otaBleService.characteristic);

    DeviceInfoBleService::setup(server)->start();

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

unsigned short BluetoothController::getDeltaTimesMTU() const
{
    const auto clientIds = extendedMetricsBleService.getDeltaTimesClientIds();

    if (clientIds.empty())
    {
        return 0;
    }

    return std::accumulate(cbegin(clientIds), cend(clientIds), 512, [&](unsigned short previousValue, unsigned short currentValue)
                           {
                    const auto currentMTU = extendedMetricsBleService.getDeltaTimesMTU(currentValue);
                    if (currentMTU == 0)
                    {
                        return previousValue;
                    }

                    return std::min(previousValue, currentMTU); });
}

bool BluetoothController::isDeltaTimesSubscribed() const
{
    return extendedMetricsBleService.deltaTimesParams.characteristic->getSubscribedCount() > 0;
}
