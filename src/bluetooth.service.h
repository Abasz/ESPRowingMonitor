#pragma once

#include <array>

#include "NimBLEDevice.h"

class BluetoothService
{
    static unsigned long const LED_BLINK_FREQUENCY = 1 * 1000 * 1000;

    inline static std::array<uint8_t, 2> const FEATURES_FLAG{0b11, 0b0};

    unsigned short const CYCLING_SPEED_CADENCE_SVC_UUID = 0x1816;
    unsigned short const CSC_MEASUREMENT_UUID = 0x2A5B;
    unsigned short const SC_CONTROL_POINT_UUID = 0x2A55;
    unsigned short const CSC_FEATURE_UUID = 0x2A5C;
    unsigned short const SENSOR_LOCATION_UUID = 0x2A5D;
    std::string DRAG_FACTOR_UUID = "CE060031-43E5-11E4-916C-0800200C9A66";
    std::string DRAG_FACTOR_SVC_UUID = "CE060030-43E5-11E4-916C-0800200C9A66";

    unsigned short const BATTERY_SVC_UUID = 0x180F;
    unsigned short const BATTERY_LEVEL_UUID = 0x2A19;

    unsigned short const DEVICE_INFO_SVC_UUID = 0x180A;
    unsigned short const MODEL_NUMBER_SVC_UUID = 0x2A24;
    unsigned short const SERIAL_NUMBER_SVC_UUID = 0x2A25;
    unsigned short const SOFTWARE_NUMBER_SVC_UUID = 0x2A28;
    unsigned short const MANUFACTURER_NAME_SVC_UUID = 0x2A29;

    unsigned short const BLE_APPEARANCE_CYCLING_SPEED_CADENCE = 1157;

    hw_timer_t *const ledTimer = timerBegin(0, 80, true);
    volatile byte ledState = HIGH;

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
    void notifyDragFactor(byte distance, byte dragFactor) const;
    bool isAnyDeviceConnected() const;
    void checkConnectedDevices();
};
