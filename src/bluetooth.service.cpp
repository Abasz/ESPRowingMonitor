#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "bluetooth.service.h"

using std::array;
using std::to_string;

BluetoothService::BluetoothService()
{
}

bool BluetoothService::isAnyDeviceConnected() const
{
    return NimBLEDevice::getServer()->getConnectedCount() > 0;
}

void BluetoothService::updateLed()
{
    // execution time: 1-5 micro sec
    // auto start = micros();
    if (isAnyDeviceConnected())
    {
        ledState = HIGH;
    }
    else
    {
        ledState = !ledState;
    }

    digitalWrite(GPIO_NUM_2, ledState);
    // auto end = micros();
    // Serial.print("led: ");
    // Serial.println(end - start);
}

void BluetoothService::setup()
{
    setupBleDevice();
    setupConnectionIndicatorLed();
}

void BluetoothService::startBLEServer() const
{
    NimBLEDevice::getAdvertising()->start();
    Log.traceln("Waiting a client connection to notify...");
}

void BluetoothService::stopServer() const
{
    NimBLEDevice::getAdvertising()->stop();
}

void BluetoothService::notifyBattery(byte batteryLevel) const
{
    batteryLevelCharacteristic->setValue(batteryLevel);
    if (batteryLevelCharacteristic->getSubscribedCount() > 0)
    {
        batteryLevelCharacteristic->notify();
    }
}

void BluetoothService::notifyDragFactor(unsigned short distance, byte dragFactor) const
{
    dragFactorCharacteristic->setValue("DF=" + to_string(dragFactor) + ", Dist=" + to_string(distance));
    if (dragFactorCharacteristic->getSubscribedCount() > 0)
    {
        dragFactorCharacteristic->notify();
    }
}

void BluetoothService::notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const
{
    if (cscMeasurementCharacteristic->getSubscribedCount() > 0)
    {
        // execution time: 11-16 microsec
        // auto start = micros();
        unsigned short revTime = lround(lastRevTime / 1000000.0 * 1024) % USHRT_MAX;
        unsigned short strokeTime = lround(lastStrokeTime / 1000000.0 * 1024) % USHRT_MAX;
        // auto stop = micros();
        // Serial.print("Time stamp calc: ");
        // Serial.println(stop - start);

        // execution time: 0-1 microsec
        // auto start = micros();
        array<uint8_t, 11> temp = {
            featuresFlag[0],

            static_cast<byte>(revCount),
            static_cast<byte>(revCount >> 8),
            static_cast<byte>(revCount >> 16),
            static_cast<byte>(revCount >> 24),

            static_cast<byte>(revTime),
            static_cast<byte>(revTime >> 8),

            static_cast<byte>(strokeCount),
            static_cast<byte>(strokeCount >> 8),
            static_cast<byte>(strokeTime),
            static_cast<byte>(strokeTime >> 8)};

        // auto stop = micros();
        // Serial.print("data calc: ");
        // Serial.println(stop - start);

        // execution time: 28-35 microsec
        // auto start = micros();
        cscMeasurementCharacteristic->setValue(temp);
        // auto stop = micros();
        // Serial.print("set value: ");
        // Serial.println(stop - start);

        // execution time: 1000-1600 microsec
        // start = micros();
        cscMeasurementCharacteristic->notify();
        // stop = micros();
        // Serial.print("notify: ");
        // Serial.println(stop - start);
    }
}

void BluetoothService::setupBleDevice()
{
    Log.traceln("Initializing BLE device");

    NimBLEDevice::init("CSC-Sensor");
    NimBLEDevice::setPower(ESP_PWR_LVL_N8);

    Log.traceln("Setting up Server");

    NimBLEDevice::createServer();

    setupServices();
    setupAdvertisment();
}

void BluetoothService::setupServices()
{
    Log.traceln("Setting up BLE Services");
    auto server = NimBLEDevice::getServer();
    auto batteryService = server->createService(batterySvcUuid);
    auto cscService = server->createService(cyclingSpeedCadenceSvcUuid);
    auto deviceInfoService = server->createService(deviceInfoSvcUuid);

    Log.traceln("Setting up BLE Characteristics");

    batteryLevelCharacteristic = batteryService->createCharacteristic(batteryLevelUuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    cscMeasurementCharacteristic = cscService->createCharacteristic(cscMeasurementUuid, NIMBLE_PROPERTY::NOTIFY);

    dragFactorCharacteristic = cscService->createCharacteristic(dragFactorUuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);

    cscService
        ->createCharacteristic(cscFeatureUuid, NIMBLE_PROPERTY::READ)
        ->setValue(featuresFlag.data(), featuresFlag.size());

    cscService
        ->createCharacteristic(sensorLocationUuid, NIMBLE_PROPERTY::READ)
        ->setValue(&featuresFlag[1], 1);

    cscService->createCharacteristic(cscControlPointUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE);

    deviceInfoService
        ->createCharacteristic(manufacturerNameSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue("ZOCO BODY FIT");
    deviceInfoService
        ->createCharacteristic(modelNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue("AR-C2");
    deviceInfoService
        ->createCharacteristic(serialNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue("20220104");
    deviceInfoService
        ->createCharacteristic(softwareNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue("0.1.0");

    Log.traceln("Starting BLE Service");

    batteryService->start();
    cscService->start();
    deviceInfoService->start();
    server->start();
}

void BluetoothService::setupAdvertisment() const
{
    auto pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAppearance(bleAppearanceCyclingSpeedCadence);
    pAdvertising->addServiceUUID(cyclingSpeedCadenceSvcUuid);
}

void BluetoothService::setupConnectionIndicatorLed() const
{
    pinMode(GPIO_NUM_2, OUTPUT);
}