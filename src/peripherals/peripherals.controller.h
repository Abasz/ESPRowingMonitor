#pragma once
#include <array>

#include "FastLED.h"

#include "bluetooth.service.h"
#include "network.service.h"
#include "sd-card.service.h"
#include "utils/EEPROM.service.h"

class PeripheralsController
{
    BluetoothService &bluetoothService;
    NetworkService &networkService;
    SdCardService &sdCardService;
    EEPROMService &eepromService;

    unsigned int lastConnectedDeviceCheckTime = 0;
    unsigned int lastBroadcastTime = 0UL;

    unsigned short bleRevTimeData = 0;
    unsigned int bleRevCountData = 0;
    unsigned short bleStrokeTimeData = 0;
    unsigned short bleStrokeCountData = 0;
    short bleAvgStrokePowerData = 0;

    std::vector<unsigned long> deltaTimes;

    unsigned char ledState = HIGH;
    CRGB::HTMLColorCode ledColor = CRGB::Black;
    inline static std::array<CRGB, 1> leds;

    void updateLed(CRGB::HTMLColorCode newLedColor);
    static void setupConnectionIndicatorLed();

public:
    PeripheralsController(BluetoothService &_bluetoothService, NetworkService &_networkService, SdCardService &sdCardService, EEPROMService &_eepromService);

    void begin();
    void update(unsigned char batteryLevel);
    void notifyBattery(unsigned char batteryLevel);
    void updateData(const RowingDataModels::RowingMetrics &data);
    void updateDeltaTime(unsigned long deltaTime);
    void notifyDragFactor(unsigned char dragFactor) const;
    bool isAnyDeviceConnected();
};
