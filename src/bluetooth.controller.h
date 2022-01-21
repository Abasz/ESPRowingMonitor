#pragma once

#include "bluetooth.service.h"

class BluetoothController
{
    BluetoothService &bluetoothService;

public:
    BluetoothController(BluetoothService &_bluetoothService);

    void begin();
    void notifyBattery(byte batteryLevel) const;
    void notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const;
    void notifyDragFactor(byte dragFactor) const;
    bool isAnyDeviceConnected() const;
};
