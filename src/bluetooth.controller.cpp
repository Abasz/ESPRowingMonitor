#include "ArduinoLog.h"

#include "bluetooth.controller.h"
#include "settings.h"

BluetoothController::BluetoothController(BluetoothService &_bluetoothService, EEPROMService &_eepromService) : bluetoothService(_bluetoothService), eepromService(_eepromService)
{
}

void BluetoothController::update()
{
    auto now = millis();
    if (now - lastConnectedDeviceCheckTime > Settings::ledBlinkFrequency)
    {
        bluetoothService.updateLed();
        lastConnectedDeviceCheckTime = now;
    }
}

void BluetoothController::begin()
{
    Log.infoln("Setting up BLE Controller");
    bluetoothService.setup();
    bluetoothService.startBLEServer();
}

bool BluetoothController::isAnyDeviceConnected() const
{
    return bluetoothService.isAnyDeviceConnected();
}

void BluetoothController::notifyBattery(byte batteryLevel) const
{
    bluetoothService.notifyBattery(batteryLevel);
}

void BluetoothController::notify(unsigned int deltaRevTime, unsigned int revCount, unsigned int deltaStrokeTime, unsigned short strokeCount, short avgStrokePower)
{
    strokeTime += lround((deltaStrokeTime / 1e6) * 1024);

    if (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService)
    {
        revTime += lround((deltaRevTime / 1e6) * 2048);
        bluetoothService.notifyPsc(revTime, revCount, strokeTime, strokeCount, avgStrokePower);
    }
    if (eepromService.getBleServiceFlag() == BleServiceFlag::CscService)
    {
        revTime += lround((deltaRevTime / 1e6) * 1024);
        bluetoothService.notifyCsc(revTime, revCount, strokeTime, strokeCount);
    }
}

void BluetoothController::notifyDragFactor(byte dragFactor) const
{
    auto distance = pow(dragFactor / 2.8, 1.0 / 3.0) * (2.0 * PI) * 10;
    bluetoothService.notifyDragFactor(static_cast<unsigned short>(distance), dragFactor);
}
