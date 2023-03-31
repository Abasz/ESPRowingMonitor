#include "ArduinoLog.h"

#include "bluetooth.controller.h"
#include "settings.h"

BluetoothController::BluetoothController(BluetoothService &_bluetoothService, NetworkService &_networkService, EEPROMService &_eepromService) : bluetoothService(_bluetoothService), networkService(_networkService), eepromService(_eepromService)
{
}

void BluetoothController::update()
{
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
    bluetoothService.setup();
    BluetoothService::startBLEServer();
    setupConnectionIndicatorLed();
}

bool BluetoothController::isAnyDeviceConnected()
{
    return BluetoothService::isAnyDeviceConnected();
}

void BluetoothController::updateLed()
{
    ledState = BluetoothController::isAnyDeviceConnected() ? HIGH : ledState == HIGH ? LOW
                                                                                     : HIGH;

    digitalWrite(GPIO_NUM_2, ledState);
}

void BluetoothController::notifyBattery(unsigned char batteryLevel) const
{
    bluetoothService.notifyBattery(batteryLevel);
}

void BluetoothController::updateData(unsigned long long revTime, unsigned int revCount, unsigned long long strokeTime, unsigned short strokeCount, short avgStrokePower)
{
    const auto secInMicroSec = 1e6L;
    revTimeData = lroundl((revTime / secInMicroSec) * (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService ? 2048 : 1024)) % USHRT_MAX;
    revCountData = revCount;
    strokeTimeData = lroundl((strokeTime / secInMicroSec) * 1024) % USHRT_MAX;
    strokeCountData = strokeCount;
    avgStrokePowerData = avgStrokePower;
}

void BluetoothController::notify() const
{
    if (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService)
    {
        bluetoothService.notifyPsc(revTimeData, revCountData, strokeTimeData, strokeCountData, avgStrokePowerData);
    }
    if (eepromService.getBleServiceFlag() == BleServiceFlag::CscService)
    {
        bluetoothService.notifyCsc(revTimeData, revCountData, strokeTimeData, strokeCountData);
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