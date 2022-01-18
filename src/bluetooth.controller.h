#pragma once

#include "bluetooth.service.h"

class BluetoothController
{
    BluetoothService bluetoothService;

public:
    BluetoothController();

    void begin() const;
    void notifyBattery(byte batteryLevel) const;
    void notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const;
    void notifyDragFactor(byte dragFactor) const;
    bool isDeviceConnected() const;
    void checkConnectedDevices();
};
