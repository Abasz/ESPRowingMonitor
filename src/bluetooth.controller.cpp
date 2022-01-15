#include <Arduino.h>

#include "bluetooth.controller.h"

BluetoothController::BluetoothController() : bluetoothService(BluetoothService())
{
}

bool BluetoothController::isDeviceConnected() const
{
    return bluetoothService.isDeviceConnected();
}

void BluetoothController::checkConnectedDevices()
{
    bluetoothService.checkConnectedDevices();
}

void BluetoothController::begin() const
{
    bluetoothService.setup();
    bluetoothService.startBLEServer();
}

void BluetoothController::setBattery(unsigned char batteryLevel) const
{
    bluetoothService.setBattery(batteryLevel);
}

void BluetoothController::notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const
{
    bluetoothService.notifyCsc(lastRevTime, revCount, lastStrokeTime, strokeCount);
}

void BluetoothController::notifyDragFactor(unsigned char dragFactor) const
{
    auto distance = pow(dragFactor / 2.8, 1.0 / 3.0) * (2.0 * PI) * 10;
    bluetoothService.notifyDragFactor(static_cast<unsigned char>(distance), dragFactor);
}
