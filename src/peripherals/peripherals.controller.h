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
    PeripheralsController(BluetoothService &_bluetoothService, NetworkService &_networkService, SdCardService &sdCardService, EEPROMService &_eepromService);

    void begin();
    void update(unsigned char batteryLevel);
    void notifyBattery(unsigned char batteryLevel);
    void updateData(const RowingDataModels::RowingMetrics &data);
    void updateDeltaTime(unsigned long deltaTime);

    /// @deprecated Standalone drag factor notification is deprecated in favor of the extended BLE API and may be removed in future
    [[deprecated("Standalone drag factor notification is deprecated in favor of the extended BLE API and may be removed in future")]]
    void notifyDragFactor(unsigned char dragFactor) const;
    bool isAnyDeviceConnected();
};
