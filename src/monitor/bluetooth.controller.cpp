#include "ArduinoLog.h"

#include "bluetooth.controller.h"
#include "settings.h"

BluetoothController::BluetoothController(BluetoothService &_bluetoothService, NetworkService &_networkService, EEPROMService &_eepromService) : bluetoothService(_bluetoothService), networkService(_networkService), eepromService(_eepromService)
{
}

void BluetoothController::update()
{
    networkService.update();
    auto now = millis();
    if (now - lastConnectedDeviceCheckTime > Settings::ledBlinkFrequency)
    {
        updateLed();
        lastConnectedDeviceCheckTime = now;
    }

    if (now - lastBroadcastTime > updateInterval)
    {
        notify();
        lastBroadcastTime = now;
    }
}

void BluetoothController::begin()
{
    Log.infoln("Setting up BLE Controller");
    NetworkService::setup();
    bluetoothService.setup();
    BluetoothService::startBLEServer();
    setupConnectionIndicatorLed();
}

bool BluetoothController::isAnyDeviceConnected()
{
    return BluetoothService::isAnyDeviceConnected() || networkService.isAnyDeviceConnected();
}

void BluetoothController::updateLed()
{
    ledState = isAnyDeviceConnected() ? HIGH : ledState == HIGH ? LOW
                                                                : HIGH;

    digitalWrite(GPIO_NUM_2, ledState);
}

void BluetoothController::notifyBattery(unsigned char batteryLevel)
{
    batteryLevelData = batteryLevel;
    bluetoothService.notifyBattery(batteryLevel);
}

void BluetoothController::updateData(RowingDataModels::RowingMetrics data)
{
    const auto secInMicroSec = 1e6L;
    bleRevTimeData = lroundl((data.lastRevTime / secInMicroSec) * (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService ? 2048 : 1024)) % USHRT_MAX;
    bleRevCountData = lround(data.distance);
    bleStrokeTimeData = lroundl((data.lastStrokeTime / secInMicroSec) * 1024) % USHRT_MAX;
    bleStrokeCountData = data.strokeCount;
    bleAvgStrokePowerData = static_cast<short>(lround(data.avgStrokePower));

    networkService.notifyClients(data, batteryLevelData, eepromService.getBleServiceFlag(), eepromService.getLogLevel());
}

void BluetoothController::notify() const
{
    if (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService)
    {
        bluetoothService.notifyPsc(bleRevTimeData, bleRevCountData, bleStrokeTimeData, bleStrokeCountData, bleAvgStrokePowerData);
    }
    if (eepromService.getBleServiceFlag() == BleServiceFlag::CscService)
    {
        bluetoothService.notifyCsc(bleRevTimeData, bleRevCountData, bleStrokeTimeData, bleStrokeCountData);
    }
}

void BluetoothController::notifyDragFactor(unsigned char dragFactor) const
{
    auto distance = pow(dragFactor / Settings::concept2MagicNumber, 1.0 / 3.0) * (2.0 * PI) * 10;
    bluetoothService.notifyDragFactor(static_cast<unsigned short>(distance), dragFactor);
}

void BluetoothController::setupConnectionIndicatorLed() const
{
    pinMode(GPIO_NUM_2, OUTPUT);
}