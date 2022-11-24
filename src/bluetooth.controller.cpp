#include "ArduinoLog.h"

#include "bluetooth.controller.h"
#include "settings.h"

BluetoothController::BluetoothController(BluetoothService &_bluetoothService) : bluetoothService(_bluetoothService)
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

void BluetoothController::notifyCsc(unsigned int deltaRevTime, unsigned int revCount, unsigned int deltaStrokeTime, unsigned short strokeCount)
{
    revTime += lround((deltaRevTime / 1e6) * 1024);
    strokeTime += lround((deltaStrokeTime / 1e6) * 1024);
    bluetoothService.notifyCsc(revTime, revCount, strokeTime, strokeCount);
}

void BluetoothController::notifyPsc(unsigned int deltaRevTime, unsigned int revCount, unsigned int deltaStrokeTime, unsigned short strokeCount, short avgStrokePower)
{
    revTime += lround((deltaRevTime / 1e6) * 2048);
    strokeTime += lround((deltaStrokeTime / 1e6) * 1024);
    bluetoothService.notifyPsc(revTime, revCount, strokeTime, strokeCount, avgStrokePower);
}

void BluetoothController::notifyDragFactor(byte dragFactor) const
{
    auto distance = pow(dragFactor / 2.8, 1.0 / 3.0) * (2.0 * PI) * 10;
    bluetoothService.notifyDragFactor(static_cast<unsigned short>(distance), dragFactor);
}
