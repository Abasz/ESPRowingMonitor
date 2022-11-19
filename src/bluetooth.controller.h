#pragma once

#include "bluetooth.service.h"

class BluetoothController
{
    BluetoothService &bluetoothService;

    unsigned int lastConnectedDeviceCheckTime = 0;

    unsigned short revTime = 0;
    unsigned short crankTime = 0;

public:
    BluetoothController(BluetoothService &_bluetoothService);

    void begin();
    void update();
    void notifyBattery(byte batteryLevel) const;
    void notifyCsc(unsigned int deltaRevTime, unsigned int revCount, unsigned int deltaStrokeTime, unsigned short strokeCount);
    void notifyPsc(unsigned int deltaRevTime, unsigned int revCount, unsigned int deltaStrokeTime, unsigned short strokeCount, short avgStrokePower);
    void notifyDragFactor(byte dragFactor) const;
    bool isAnyDeviceConnected() const;
};
