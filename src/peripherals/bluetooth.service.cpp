#include <string>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../utils/configuration.h"
#include "bluetooth.service.h"

using std::array;
using std::to_string;

BluetoothService::ControlPointCallbacks::ControlPointCallbacks(BluetoothService &_bleService) : bleService(_bleService) {}

BluetoothService::HandleForcesCallbacks::HandleForcesCallbacks(BluetoothService &_bleService) : bleService(_bleService) {}

BluetoothService::ServerCallbacks::ServerCallbacks() {}

void BluetoothService::ServerCallbacks::onConnect(NimBLEServer *pServer)
{
    if (NimBLEDevice::getServer()->getConnectedCount() < 2)
    {
        Log.verboseln("Device connected");
        NimBLEDevice::getAdvertising()->start();
    }
}

void BluetoothService::HandleForcesCallbacks::onSubscribe(NimBLECharacteristic *const pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue)
{
    if (pCharacteristic->getUUID().toString() == CommonBleFlags::handleForcesUuid)
    {
        bleService.handleForcesClientId = desc->conn_handle;
    }
}

void BluetoothService::ControlPointCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic)
{
    NimBLEAttValue message = pCharacteristic->getValue();

    Log.verboseln("Incoming connection");

    if (message.length() == 0)
    {
        Log.infoln("Invalid request, no Op Code");
        array<uint8_t, 3> errorResponse = {
            static_cast<unsigned char>(PSCOpCodes::ResponseCode),
            static_cast<unsigned char>(0),
            static_cast<unsigned char>(PSCResponseOpCodes::OperationFailed)};
        pCharacteristic->setValue(errorResponse);
        pCharacteristic->indicate();

        return;
    }

    Log.infoln("Op Code: %d; Length: %d", message[0], message.length());

    switch (message[0])
    {

    case static_cast<int>(PSCOpCodes::SetLogLevel):
    {
        Log.infoln("Set LogLevel");

        auto response = PSCResponseOpCodes::InvalidParameter;
        if (message.length() == 2 && message[1] >= 0 && message[1] <= 6)
        {
            Log.infoln("New LogLevel: %d", message[1]);
            bleService.eepromService.setLogLevel(static_cast<ArduinoLogLevel>(message[1]));
            response = PSCResponseOpCodes::Successful;
        }

        array<uint8_t, 3>
            temp = {
                static_cast<unsigned char>(PSCOpCodes::ResponseCode),
                static_cast<unsigned char>(message[0]),
                static_cast<unsigned char>(response)};

        pCharacteristic->setValue(temp);
    }
    break;

    case static_cast<int>(PSCOpCodes::ChangeBleService):
    {
        Log.infoln("Change BLE Service");

        if (message.length() == 2 && message[1] >= 0 && message[1] <= 1)
        {
            Log.infoln("New BLE Service: %s", message[1] == static_cast<unsigned char>(BleServiceFlag::CscService) ? "CSC" : "CPS");
            bleService.eepromService.setBleServiceFlag(static_cast<BleServiceFlag>(message[1]));
            array<uint8_t, 3> temp = {
                static_cast<unsigned char>(PSCOpCodes::ResponseCode),
                static_cast<unsigned char>(message[0]),
                static_cast<unsigned char>(PSCResponseOpCodes::Successful)};
            pCharacteristic->setValue(temp);
            pCharacteristic->indicate();

            Log.verboseln("Restarting device in 5s");
            delay(100);
            esp_restart();

            break;
        }

        array<uint8_t, 3> temp = {
            static_cast<unsigned char>(PSCOpCodes::ResponseCode),
            static_cast<unsigned char>(message[0]),
            static_cast<unsigned char>(PSCResponseOpCodes::InvalidParameter)};

        pCharacteristic->setValue(temp);
    }
    break;

    default:
    {
        Log.infoln("Not Supported Op Code: %d", message[0]);
        array<uint8_t, 3> response = {
            static_cast<unsigned char>(PSCOpCodes::ResponseCode),
            static_cast<unsigned char>(message[0]),
            static_cast<unsigned char>(PSCResponseOpCodes::UnsupportedOpCode)};
        pCharacteristic->setValue(response);
    }
    break;
    }

    Log.verboseln("Send indicate");
    pCharacteristic->indicate();
}

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
    std::string value = "DF=" + to_string(dragFactor) + ", Dist=" + to_string(distance);
    dragFactorCharacteristic->setValue(value);
    if (dragFactorCharacteristic->getSubscribedCount() > 0)
    {
        dragFactorCharacteristic->notify();
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

    const unsigned char settings = ((Configurations::enableWebSocketDeltaTimeLogging ? static_cast<unsigned char>(eepromService.getLogToWebsocket()) + 1 : 0) << 0U) | ((Configurations::supportSdCardLogging && sdCardService.isLogFileOpen() ? static_cast<unsigned char>(eepromService.getLogToSdCard()) + 1 : 0) << 2U) | (static_cast<unsigned char>(eepromService.getLogLevel()) << 4U);

    const auto length = 12U;
    array<uint8_t, length> temp = {
        settings,

        static_cast<unsigned char>(avgStrokePower),
        static_cast<unsigned char>(avgStrokePower >> 8),

        static_cast<unsigned char>(recoveryDuration),
        static_cast<unsigned char>(recoveryDuration >> 8),
        static_cast<unsigned char>(recoveryDuration >> 16),
        static_cast<unsigned char>(recoveryDuration >> 24),
        static_cast<unsigned char>(driveDuration),
        static_cast<unsigned char>(driveDuration >> 8),
        static_cast<unsigned char>(driveDuration >> 16),
        static_cast<unsigned char>(driveDuration >> 24),

        dragFactor,
    };

    extendedMetricsCharacteristic->setValue(temp);
    extendedMetricsCharacteristic->notify();
}

void BluetoothService::notifyHandleForces(const std::vector<float> &handleForces) const
{
    if (handleForcesCharacteristic->getSubscribedCount() == 0)
    {
        return;
    }

    const auto mtu = handleForcesCharacteristic->getService()->getServer()->getPeerMTU(handleForcesClientId);

    const unsigned char chunkSize = (mtu - 3 - 2) / sizeof(float);
    const unsigned char split = handleForces.size() / chunkSize + (handleForces.size() % chunkSize == 0 ? 0 : 1);

    auto i = 0;
    Log.traceln("MTU of extended: %d, chunk size(bytes): %d, number of chunks: %d\n", mtu, chunkSize, split);

    while (i < split)
    {
        auto start = handleForces.cbegin() + i * chunkSize;
        const auto end = (i + 1) * chunkSize < handleForces.size() ? chunkSize * sizeof(float) : (handleForces.size() - i * chunkSize) * sizeof(float);
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
    array<uint8_t, length> temp = {
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
    array<uint8_t, length> temp = {
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