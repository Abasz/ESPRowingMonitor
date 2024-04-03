#include <string>

#include "NimBLEDevice.h"

#include "../utils/configuration.h"
#include "bluetooth.service.h"

BluetoothService::BluetoothService(EEPROMService &_eepromService, SdCardService &_sdCardService) : eepromService(_eepromService), sdCardService(_sdCardService), controlPointCallbacks(*this), handleForcesCallbacks(*this)
{
}

bool BluetoothService::isAnyDeviceConnected()
{
    return NimBLEDevice::getServer()->getConnectedCount() > 0;
}

void BluetoothService::setup()
{
    setupBleDevice();
    BluetoothService::startBLEServer();
}

void BluetoothService::startBLEServer()
{
    NimBLEDevice::getAdvertising()->start();
    Log.verboseln("Waiting a client connection to notify...");
}

void BluetoothService::stopServer()
{
    NimBLEDevice::getAdvertising()->stop();
}

void BluetoothService::setupBleDevice()
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

void BluetoothService::setupServices()
{
    Log.verboseln("Setting up BLE Services");
    auto *server = NimBLEDevice::getServer();

    if constexpr (Configurations::batteryPinNumber != GPIO_NUM_NC)
    {
        auto *batteryService = server->createService(CommonBleFlags::batterySvcUuid);

        batteryLevelCharacteristic = batteryService->createCharacteristic(CommonBleFlags::batteryLevelUuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
        batteryService->start();
    }

    auto *deviceInfoService = server->createService(CommonBleFlags::deviceInfoSvcUuid);

    auto *measurementService = eepromService.getBleServiceFlag() == BleServiceFlag::CscService ? setupCscServices(server) : setupPscServices(server);

    Log.verboseln("Setting up BLE Characteristics");

    deviceInfoService
        ->createCharacteristic(CommonBleFlags::manufacturerNameSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::deviceName);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::modelNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::modelNumber);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::serialNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::serialNumber);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::softwareNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::softwareVersion);

    Log.verboseln("Starting BLE Service");

    measurementService->start();
    deviceInfoService->start();
    server->start();
}

NimBLEService *BluetoothService::setupCscServices(NimBLEServer *const server)
{
    Log.infoln("Setting up Cycling Speed and Cadence Profile");

    auto *cscService = server->createService(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid);
    cscMeasurementCharacteristic = cscService->createCharacteristic(CSCSensorBleFlags::cscMeasurementUuid, NIMBLE_PROPERTY::NOTIFY);

    if constexpr (Configurations::hasExtendedBleMetrics)
    {
        handleForcesCharacteristic = cscService->createCharacteristic(CommonBleFlags::handleForcesUuid, NIMBLE_PROPERTY::NOTIFY);
        handleForcesCharacteristic->setCallbacks(&handleForcesCallbacks);

        extendedMetricsCharacteristic = cscService->createCharacteristic(CommonBleFlags::extendedMetricsUuid, NIMBLE_PROPERTY::NOTIFY);
    }
    else
    {
        dragFactorCharacteristic = cscService->createCharacteristic(CommonBleFlags::dragFactorUuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    }

    cscService
        ->createCharacteristic(CSCSensorBleFlags::cscFeatureUuid, NIMBLE_PROPERTY::READ)
        ->setValue(CSCSensorBleFlags::cscFeaturesFlag);

    cscService
        ->createCharacteristic(CommonBleFlags::sensorLocationUuid, NIMBLE_PROPERTY::READ)
        ->setValue(CommonBleFlags::sensorLocationFlag);

    cscService->createCharacteristic(CSCSensorBleFlags::cscControlPointUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE)->setCallbacks(&controlPointCallbacks);

    return cscService;
}

NimBLEService *BluetoothService::setupPscServices(NimBLEServer *const server)
{
    Log.infoln("Setting up Cycling Power Profile");
    auto *pscService = server->createService(PSCSensorBleFlags::cyclingPowerSvcUuid);
    pscMeasurementCharacteristic = pscService->createCharacteristic(PSCSensorBleFlags::pscMeasurementUuid, NIMBLE_PROPERTY::NOTIFY);

    if constexpr (Configurations::hasExtendedBleMetrics)
    {
        handleForcesCharacteristic = pscService->createCharacteristic(CommonBleFlags::handleForcesUuid, NIMBLE_PROPERTY::NOTIFY);
        handleForcesCharacteristic->setCallbacks(&handleForcesCallbacks);

        extendedMetricsCharacteristic = pscService->createCharacteristic(CommonBleFlags::extendedMetricsUuid, NIMBLE_PROPERTY::NOTIFY);
    }
    else
    {
        dragFactorCharacteristic = pscService->createCharacteristic(CommonBleFlags::dragFactorUuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    }

    pscService
        ->createCharacteristic(PSCSensorBleFlags::pscFeatureUuid, NIMBLE_PROPERTY::READ)
        ->setValue(PSCSensorBleFlags::pscFeaturesFlag);

    pscService
        ->createCharacteristic(CommonBleFlags::sensorLocationUuid, NIMBLE_PROPERTY::READ)
        ->setValue(CommonBleFlags::sensorLocationFlag);

    pscService->createCharacteristic(PSCSensorBleFlags::pscControlPointUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE)->setCallbacks(&controlPointCallbacks);

    return pscService;
}

void BluetoothService::setupAdvertisement() const
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