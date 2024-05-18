#include <array>
#include <numeric>
#include <string>

#include "NimBLEDevice.h"

#include "../utils/configuration.h"
#include "./bluetooth.service.h"

BluetoothService::BluetoothService(EEPROMService &_eepromService, SdCardService &_sdCardService) : eepromService(_eepromService), sdCardService(_sdCardService), controlPointCallbacks(*this), chunkedNotifyMetricCallbacks(*this), serverCallbacks(*this)
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

    auto *measurementService = eepromService.getBleServiceFlag() == BleServiceFlag::CscService ? setupCscServices(server) : setupPscServices(server);
    measurementService->start();

    if constexpr (Configurations::hasExtendedBleMetrics)
    {
        auto *extendedMetricsService = setupExtendedMetricsServices(server);
        extendedMetricsService->start();
    }

    auto *settingsService = setupSettingsServices(server);
    settingsService->start();
    auto *deviceInfoService = setupDeviceInfoServices(server);
    deviceInfoService->start();

    Log.verboseln("Starting BLE Server");

    server->start();
}

NimBLEService *BluetoothService::setupCscServices(NimBLEServer *const server)
{
    Log.infoln("Setting up Cycling Speed and Cadence Profile");

    auto *cscService = server->createService(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid);
    baseMetricsParameters.characteristic = cscService->createCharacteristic(CSCSensorBleFlags::cscMeasurementUuid, NIMBLE_PROPERTY::NOTIFY);

    if constexpr (!Configurations::hasExtendedBleMetrics)
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
    baseMetricsParameters.characteristic = pscService->createCharacteristic(PSCSensorBleFlags::pscMeasurementUuid, NIMBLE_PROPERTY::NOTIFY);

    if constexpr (!Configurations::hasExtendedBleMetrics)
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

NimBLEService *BluetoothService::setupExtendedMetricsServices(NimBLEServer *const server)
{
    Log.infoln("Setting up Extended Metrics Services");
    auto *extendedMetricsService = server->createService(CommonBleFlags::extendedMetricsServiceUuid);

    handleForcesParameters.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::handleForcesUuid, NIMBLE_PROPERTY::NOTIFY);
    handleForcesParameters.characteristic->setCallbacks(&chunkedNotifyMetricCallbacks);
    deltaTimesParameters.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::deltaTimesUuid, NIMBLE_PROPERTY::NOTIFY);
    deltaTimesParameters.characteristic->setCallbacks(&chunkedNotifyMetricCallbacks);

    extendedMetricsParameters.characteristic = extendedMetricsService->createCharacteristic(CommonBleFlags::extendedMetricsUuid, NIMBLE_PROPERTY::NOTIFY);

    return extendedMetricsService;
}

NimBLEService *BluetoothService::setupDeviceInfoServices(NimBLEServer *const server)
{
    Log.traceln("Setting up Device Info Service");
    auto *deviceInfoService = server->createService(CommonBleFlags::deviceInfoSvcUuid);

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

    return deviceInfoService;
}

NimBLEService *BluetoothService::setupSettingsServices(NimBLEServer *const server)
{
    Log.traceln("Setting up Settings Service");
    auto *settingsService = server->createService(CommonBleFlags::settingsServiceUuid);
    settingsCharacteristic = settingsService->createCharacteristic(CommonBleFlags::settingsUuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    settingsCharacteristic->setValue(getSettings());

    settingsService->createCharacteristic(CommonBleFlags::settingsControlPointUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE)->setCallbacks(&controlPointCallbacks);

    return settingsService;
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

unsigned short BluetoothService::getDeltaTimesMTU() const
{
    if (deltaTimesParameters.characteristic->getSubscribedCount() == 0)
    {
        return 0;
    }

    return std::accumulate(deltaTimesParameters.clientIds.cbegin(), deltaTimesParameters.clientIds.cend(), 512, [&](unsigned short previousValue, unsigned short currentValue)
                           {
                    const auto currentMTU = deltaTimesParameters.characteristic->getService()->getServer()->getPeerMTU(currentValue);
                    if (currentMTU == 0)
                    {
                        return previousValue;
                    }

                    return std::min(previousValue, currentMTU); });
}

bool BluetoothService::isDeltaTimesSubscribed() const
{
    return deltaTimesParameters.characteristic->getSubscribedCount() > 0;
}

std::array<unsigned char, 1U> BluetoothService::getSettings() const
{
    const unsigned char settings =
        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(eepromService.getLogToBluetooth()) + 1 : 0) << 0U) |
        ((Configurations::supportSdCardLogging && sdCardService.isLogFileOpen() ? static_cast<unsigned char>(eepromService.getLogToSdCard()) + 1 : 0) << 2U) |
        (static_cast<unsigned char>(eepromService.getLogLevel()) << 4U);

    std::array<unsigned char, settingsArrayLength> temp = {
        settings,
    };

    return temp;
}