#pragma once

#include <array>

#include "NimBLEDevice.h"

class BluetoothService
{
    inline static std::array<uint8_t, 2> const FEATURES_FLAG{0b11, 0b0};

    static unsigned short const CYCLING_SPEED_CADENCE_SVC_UUID = 0x1816;
    static unsigned short const CSC_MEASUREMENT_UUID = 0x2A5B;
    static unsigned short const SC_CONTROL_POINT_UUID = 0x2A55;
    static unsigned short const CSC_FEATURE_UUID = 0x2A5C;
    static unsigned short const SENSOR_LOCATION_UUID = 0x2A5D;
    inline static std::string const DRAG_FACTOR_UUID = "CE060031-43E5-11E4-916C-0800200C9A66";
    inline static std::string const DRAG_FACTOR_SVC_UUID = "CE060030-43E5-11E4-916C-0800200C9A66";

    static unsigned short const BATTERY_SVC_UUID = 0x180F;
    static unsigned short const BATTERY_LEVEL_UUID = 0x2A19;

    static unsigned short const DEVICE_INFO_SVC_UUID = 0x180A;
    static unsigned short const MODEL_NUMBER_SVC_UUID = 0x2A24;
    static unsigned short const SERIAL_NUMBER_SVC_UUID = 0x2A25;
    static unsigned short const SOFTWARE_NUMBER_SVC_UUID = 0x2A28;
    static unsigned short const MANUFACTURER_NAME_SVC_UUID = 0x2A29;

    static unsigned short const BLE_APPEARANCE_CYCLING_SPEED_CADENCE = 1157;

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
