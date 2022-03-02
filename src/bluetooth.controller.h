#pragma once

#include "bluetooth.service.h"

class BluetoothController
{
    BluetoothService &bluetoothService;

    unsigned long lastConnectedDeviceCheckTime = 0;

public:
    BluetoothController(BluetoothService &_bluetoothService);

    void begin();
    void update();
    void notifyBattery(byte batteryLevel) const;
    void notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const;
    void notifyDragFactor(byte dragFactor) const;
    bool isAnyDeviceConnected() const;
};
