#pragma once

#include "bluetooth.service.h"
#include "network.service.h"
#include "utils/EEPROM.service.h"

class PeripheralsController
{
    BluetoothService &bluetoothService;
    NetworkService &networkService;
    EEPROMService &eepromService;

    static const unsigned int updateInterval = 1000;

    unsigned int lastConnectedDeviceCheckTime = 0;
    unsigned int lastBroadcastTime = 0UL;

    unsigned char batteryLevelData = 0;
    unsigned short bleRevTimeData = 0;
    unsigned int bleRevCountData = 0;
    unsigned short bleStrokeTimeData = 0;
    unsigned short bleStrokeCountData = 0;
    short bleAvgStrokePowerData = 0;

    unsigned char ledState = HIGH;

    void notify() const;
    void updateLed();
    void setupConnectionIndicatorLed() const;

public:
    PeripheralsController(BluetoothService &_bluetoothService, NetworkService &_networkService, EEPROMService &_eepromService);

    void begin();
    void update();
    void notifyBattery(unsigned char batteryLevel);
    void updateData(const RowingDataModels::RowingMetrics data);
    void notifyDragFactor(unsigned char dragFactor) const;
    bool isAnyDeviceConnected();
};
