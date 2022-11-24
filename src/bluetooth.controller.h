#pragma once

#include "EEPROM.service.h"
#include "bluetooth.service.h"

class BluetoothController
{
    BluetoothService &bluetoothService;
    EEPROMService &eepromService;

    unsigned int lastConnectedDeviceCheckTime = 0;

    unsigned short revTime = 0;
    unsigned short strokeTime = 0;

public:
    BluetoothController(BluetoothService &_bluetoothService, EEPROMService &_eepromService);

    void begin();
    void update();
    void notifyBattery(byte batteryLevel) const;
    void notify(unsigned int deltaRevTime, unsigned int revCount, unsigned int deltaStrokeTime, unsigned short strokeCount, short avgStrokePower);
    void notifyDragFactor(byte dragFactor) const;
    bool isAnyDeviceConnected() const;
};
