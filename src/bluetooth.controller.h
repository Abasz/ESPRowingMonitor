#pragma once

#include "EEPROM.service.h"
#include "bluetooth.service.h"

class BluetoothController
{
    BluetoothService &bluetoothService;
    EEPROMService &eepromService;

    unsigned int lastConnectedDeviceCheckTime = 0;

public:
    BluetoothController(BluetoothService &_bluetoothService, EEPROMService &_eepromService);

    void begin();
    void update();
    void notifyBattery(byte batteryLevel) const;
    void notify(unsigned long long revTime, unsigned int revCount, unsigned long long strokeTime, unsigned short strokeCount, short avgStrokePower);
    void notifyDragFactor(byte dragFactor) const;
    bool isAnyDeviceConnected() const;
};
