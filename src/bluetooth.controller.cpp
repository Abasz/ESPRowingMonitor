#include "ArduinoLog.h"

#include "bluetooth.controller.h"
#include "settings.h"

BluetoothController::BluetoothController(BluetoothService &_bluetoothService) : bluetoothService(_bluetoothService)
{
}

void BluetoothController::update(byte batteryLevel)
{
    auto now = millis();
    if (now - lastConnectedDeviceCheckTime > Settings::ledBlinkFrequency)
    {
        auto ledColor = CRGB::Blue;
        if (batteryLevel < 30)
        {
            ledColor = CRGB::Red;
        }
        if (batteryLevel > 80)
        {
            ledColor = CRGB::Green;
        }
        bluetoothService.updateLed(ledColor);
        lastConnectedDeviceCheckTime = now;
    }
}

void BluetoothController::begin()
{
    Log.infoln("Setting up BLE Controller");
    bluetoothService.setup();
    // bluetoothService.startBLEServer();
}

bool BluetoothController::isAnyDeviceConnected() const
{
    return bluetoothService.isAnyDeviceConnected();
}

void BluetoothController::notifyBattery(byte batteryLevel) const
{
    bluetoothService.notifyBattery(batteryLevel);
}

void BluetoothController::notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const
{
    bluetoothService.notifyCsc(lastRevTime, revCount, lastStrokeTime, strokeCount);
}

void BluetoothController::notifyPsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount, short avgStrokePower) const
{
    bluetoothService.notifyPsc(lastRevTime, revCount, lastStrokeTime, strokeCount, avgStrokePower);
}

void BluetoothController::notifyDragFactor(byte dragFactor) const
{
    auto distance = pow(dragFactor / 2.8, 1.0 / 3.0) * (2.0 * PI) * 10;
    bluetoothService.notifyDragFactor(static_cast<unsigned short>(distance), dragFactor);
}
