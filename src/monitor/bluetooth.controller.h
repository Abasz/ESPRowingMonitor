#pragma once

#include "bluetooth.service.h"
#include "utils/EEPROM.service.h"

class BluetoothController
{
    BluetoothService &bluetoothService;
    EEPROMService &eepromService;

    unsigned int lastConnectedDeviceCheckTime = 0;
    unsigned int lastBroadcastTime = 0UL;

    unsigned short revTimeData = 0;
    unsigned int revCountData = 0;
    unsigned short strokeTimeData = 0;
    unsigned short strokeCountData = 0;
    short avgStrokePowerData = 0;

    void notify();

public:
    BluetoothController(BluetoothService &_bluetoothService, EEPROMService &_eepromService);

    void begin();
    void update();
    void notifyBattery(unsigned char batteryLevel) const;
    void updateData(unsigned long long revTime, unsigned int revCount, unsigned long long strokeTime, unsigned short strokeCount, short avgStrokePower = 0);
    void notifyDragFactor(unsigned char dragFactor) const;
    bool isAnyDeviceConnected() const;
};