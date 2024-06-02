#pragma once
#include <array>

#include "FastLED.h"

#include "../rower/stroke.model.h"
#include "../utils/EEPROM.service.h"
#include "./bluetooth.service.h"
#include "./sd-card.service.h"

class PeripheralsController
{
    BluetoothService &bluetoothService;
    SdCardService &sdCardService;
    EEPROMService &eepromService;

    unsigned int lastConnectedDeviceCheckTime = 0;
    unsigned int lastMetricsBroadcastTime = 0UL;
    unsigned int lastDeltaTimesBroadcastTime = 0UL;

    unsigned short bleRevTimeData = 0;
    unsigned int bleRevCountData = 0;
    unsigned short bleStrokeTimeData = 0;
    unsigned short bleStrokeCountData = 0;
    short bleAvgStrokePowerData = 0;

    std::vector<unsigned long> sdDeltaTimes;
    std::vector<unsigned long> bleDeltaTimes;

    unsigned char ledState = HIGH;
    CRGB::HTMLColorCode ledColor = CRGB::Black;
    inline static std::array<CRGB, 1> leds;

    void updateLed(CRGB::HTMLColorCode newLedColor);
    void flushBleDeltaTimes(unsigned short mtu);
    static void setupConnectionIndicatorLed();

public:
    PeripheralsController(BluetoothService &_bluetoothService, SdCardService &sdCardService, EEPROMService &_eepromService);

    void begin();
    void update(unsigned char batteryLevel);
    void notifyBattery(unsigned char batteryLevel);
    void updateData(const RowingDataModels::RowingMetrics &data);
    void updateDeltaTime(unsigned long deltaTime);
    bool isAnyDeviceConnected();
};
