#pragma once
#include <array>
#include <vector>

#include "FastLED.h"

#include "../utils/EEPROM.service.interface.h"
#include "./bluetooth/bluetooth.controller.interface.h"
#include "./peripherals.controller.interface.h"
#include "./sd-card.service.interface.h"

using std::vector;

class PeripheralsController final : public IPeripheralsController
{
    IBluetoothController &bluetoothController;
    ISdCardService &sdCardService;
    IEEPROMService &eepromService;

    unsigned int lastConnectedDeviceCheckTime = 0;
    unsigned int lastMetricsBroadcastTime = 0UL;
    unsigned int lastDeltaTimesBroadcastTime = 0UL;

    unsigned short bleRevTimeData = 0;
    unsigned int bleRevCountData = 0;
    unsigned short bleStrokeTimeData = 0;
    unsigned short bleStrokeCountData = 0;
    short bleAvgStrokePowerData = 0;

    vector<unsigned long> sdDeltaTimes;
    vector<unsigned long> bleDeltaTimes;

    unsigned char ledState = HIGH;
    CRGB::HTMLColorCode ledColor = CRGB::Black;
    inline static std::array<CRGB, 1> leds;

    void updateLed(CRGB::HTMLColorCode newLedColor);
    void flushBleDeltaTimes(unsigned short mtu);
    static void setupConnectionIndicatorLed();

public:
    PeripheralsController(IBluetoothController &_bluetoothController, ISdCardService &sdCardService, IEEPROMService &_eepromService);

    void begin() override;
    void update(unsigned char batteryLevel) override;
    void notifyBattery(unsigned char batteryLevel) override;
    void updateData(const RowingDataModels::RowingMetrics &data) override;
    void updateDeltaTime(unsigned long deltaTime) override;
    bool isAnyDeviceConnected() override;
};
