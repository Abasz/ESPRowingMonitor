#pragma once

#include <array>

#include "NimBLEDevice.h"

class BluetoothService
{
    inline static std::array<uint8_t, 2> const featuresFlag{0b11, 0b0};

    static unsigned short const cyclingSpeedCadenceSvcUuid = 0x1816;
    static unsigned short const cscMeasurementUuid = 0x2A5B;
    static unsigned short const cscControlPointUuid = 0x2A55;
    static unsigned short const cscFeatureUuid = 0x2A5C;
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

    byte ledState = HIGH;

    NimBLECharacteristic *batteryLevelCharacteristic;
    NimBLECharacteristic *cscMeasurementCharacteristic;
    NimBLECharacteristic *dragFactorCharacteristic;

    void setupBleDevice();
    void setupServices();
    void setupAdvertisment() const;
    void setupConnectionIndicatorLed() const;

public:
    BluetoothService();

    void setup();
    void startBLEServer() const;
    void stopServer() const;
    void notifyBattery(byte batteryLevel) const;
    void notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const;
    void notifyDragFactor(unsigned short distance, byte dragFactor) const;
    bool isAnyDeviceConnected() const;
    void updateLed();
};
