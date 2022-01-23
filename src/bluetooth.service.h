#pragma once

#include <array>

#include "NimBLEDevice.h"
#include "FastLED.h"

class BluetoothService
{
    static byte constexpr sensorLocationFlag = 0b0;

    static byte const cscMeasurementFeaturesFlag = 0b11;
    static unsigned short constexpr cscFeaturesFlag = 0b11;
    static unsigned short const cyclingSpeedCadenceSvcUuid = 0x1816;
    static unsigned short const cscMeasurementUuid = 0x2A5B;
    static unsigned short const cscControlPointUuid = 0x2A55;
    static unsigned short const cscFeatureUuid = 0x2A5C;

    static unsigned short const pscMeasurementFeaturesFlag = 0b110000;
    static unsigned int constexpr pscFeaturesFlag = 0b1100;
    static unsigned short const cyclingPowerSvcUuid = 0x1818;
    static unsigned short const pscMeasurementUuid = 0x2A63;
    static unsigned short const pscControlPointUuid = 0x2A66;
    static unsigned short const pscFeatureUuid = 0x2A65;

    static unsigned short const sensorLocationUuid = 0x2A5D;
    inline static std::string const dragFactorUuid = "CE060031-43E5-11E4-916C-0800200C9A66";
    inline static std::string const dragFactorSvcUuid = "CE060030-43E5-11E4-916C-0800200C9A66";

    static unsigned short const batterySvcUuid = 0x180F;
    static unsigned short const batteryLevelUuid = 0x2A19;

    static unsigned short const deviceInfoSvcUuid = 0x180A;
    static unsigned short const modelNumberSvcUuid = 0x2A24;
    static unsigned short const serialNumberSvcUuid = 0x2A25;
    static unsigned short const softwareNumberSvcUuid = 0x2A28;
    static unsigned short const manufacturerNameSvcUuid = 0x2A29;

    static unsigned short const bleAppearanceCyclingSpeedCadence = 1157;
    static unsigned short const bleAppearanceCyclingPower = 1156;

    CRGB::HTMLColorCode ledColor = CRGB::Black;
    inline static CRGB leds[1];

    NimBLECharacteristic *batteryLevelCharacteristic;
    NimBLECharacteristic *cscMeasurementCharacteristic;
    NimBLECharacteristic *pscMeasurementCharacteristic;
    NimBLECharacteristic *dragFactorCharacteristic;

    void setupBleDevice();
    void setupServices();
    NimBLEService *setupCscServices(NimBLEServer *server);
    NimBLEService *setupPscServices(NimBLEServer *server);
    void setupAdvertisement() const;
    void setupConnectionIndicatorLed() const;

public:
    BluetoothService();

    void setup();
    void startBLEServer() const;
    void stopServer() const;
    void notifyBattery(byte batteryLevel) const;
    void notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const;
    void notifyPsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount, short avgStrokePower) const;
    void notifyDragFactor(unsigned short distance, byte dragFactor) const;
    bool isAnyDeviceConnected() const;
    void updateLed(CRGB::HTMLColorCode newLedColor);
};
