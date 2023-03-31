#pragma once

#include "bluetooth.service.h"
#include "network.service.h"
#include "utils/EEPROM.service.h"

class BluetoothController
{
    BluetoothService &bluetoothService;
    NetworkService &networkService;
    EEPROMService &eepromService;

    static const unsigned int updateInterval = 1000;

    unsigned int lastConnectedDeviceCheckTime = 0;
    unsigned int lastBroadcastTime = 0UL;

    unsigned short revTimeData = 0;
    unsigned int revCountData = 0;
    unsigned short strokeTimeData = 0;
    unsigned short strokeCountData = 0;
    short avgStrokePowerData = 0;

    unsigned char ledState = HIGH;

    void notify() const;
    void updateLed();
    void setupConnectionIndicatorLed() const;

public:
    BluetoothController(BluetoothService &_bluetoothService, NetworkService &_networkService, EEPROMService &_eepromService);

    void begin();
    void update();
    void notifyBattery(unsigned char batteryLevel) const;
    void updateData(unsigned long long revTime, unsigned int revCount, unsigned long long strokeTime, unsigned short strokeCount, short avgStrokePower = 0);
    void notifyDragFactor(unsigned char dragFactor) const;
    static bool isAnyDeviceConnected();
};
