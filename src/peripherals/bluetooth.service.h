#pragma once

#include <array>

#include "NimBLEDevice.h"

#include "../utils/EEPROM.service.h"
#include "../utils/enums.h"

class BluetoothService
{
    class ControlPointCallbacks : public NimBLECharacteristicCallbacks
    {
        BluetoothService &bleService;

    public:
        explicit ControlPointCallbacks(BluetoothService &_bleService);

        void onWrite(NimBLECharacteristic *pCharacteristic) override;
    };

    EEPROMService &eepromService;
    ControlPointCallbacks controlPointCallbacks;

    NimBLECharacteristic *batteryLevelCharacteristic = nullptr;
    NimBLECharacteristic *cscMeasurementCharacteristic = nullptr;
    NimBLECharacteristic *pscMeasurementCharacteristic = nullptr;
    NimBLECharacteristic *dragFactorCharacteristic = nullptr;

    void setupBleDevice();
    void setupServices();
    NimBLEService *setupCscServices(NimBLEServer *server);
    NimBLEService *setupPscServices(NimBLEServer *server);
    void setupAdvertisement() const;

public:
    explicit BluetoothService(EEPROMService &_eepromService);

    void setup();
    static void startBLEServer();
    static void stopServer();
    void notifyBattery(unsigned char batteryLevel) const;
    void notifyCsc(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount) const;
    void notifyPsc(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) const;
    void notifyDragFactor(unsigned short distance, unsigned char dragFactor) const;
    static bool isAnyDeviceConnected();
};
