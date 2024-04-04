#pragma once

#include <array>

#include "NimBLEDevice.h"

#include "../utils/EEPROM.service.h"
#include "../utils/enums.h"
#include "./sd-card.service.h"

class BluetoothService
{
    class ControlPointCallbacks : public NimBLECharacteristicCallbacks
    {
        BluetoothService &bleService;

    public:
        explicit ControlPointCallbacks(BluetoothService &_bleService);

        void onWrite(NimBLECharacteristic *pCharacteristic) override;
    };

    class HandleForcesCallbacks : public NimBLECharacteristicCallbacks
    {
        BluetoothService &bleService;

    public:
        explicit HandleForcesCallbacks(BluetoothService &_bleService);

        void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue) override;
    };

    class ServerCallbacks : public NimBLEServerCallbacks
    {
        BluetoothService &bleService;

    public:
        explicit ServerCallbacks(BluetoothService &_bleService);

        void onConnect(NimBLEServer *pServer) override;
        void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override;
    };

    EEPROMService &eepromService;
    SdCardService &sdCardService;
    ControlPointCallbacks controlPointCallbacks;
    HandleForcesCallbacks handleForcesCallbacks;
    ServerCallbacks serverCallbacks;

    NimBLECharacteristic *batteryLevelCharacteristic = nullptr;
    NimBLECharacteristic *cscMeasurementCharacteristic = nullptr;
    NimBLECharacteristic *pscMeasurementCharacteristic = nullptr;
    NimBLECharacteristic *dragFactorCharacteristic = nullptr;
    NimBLECharacteristic *handleForcesCharacteristic = nullptr;
    NimBLECharacteristic *extendedMetricsCharacteristic = nullptr;
    NimBLECharacteristic *settingsCharacteristic = nullptr;

    vector<unsigned char> handleForcesClientIds;
    static const unsigned char settingsArrayLength = 1U;

    void notifyCsc(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount) const;
    void notifyPsc(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) const;

    void setupBleDevice();
    void setupServices();
    void setupAdvertisement() const;

    NimBLEService *setupCscServices(NimBLEServer *server);
    NimBLEService *setupPscServices(NimBLEServer *server);
    NimBLEService *setupExtendedMetricsServices(NimBLEServer *server);
    NimBLEService *setupSettingsServices(NimBLEServer *server);
    static NimBLEService *setupDeviceInfoServices(NimBLEServer *server);

    std::array<unsigned char, settingsArrayLength> getSettings() const;

public:
    explicit BluetoothService(EEPROMService &_eepromService, SdCardService &_sdCardService);

    void setup();
    static void startBLEServer();
    static void stopServer();
    void notifyBattery(unsigned char batteryLevel) const;

    void notifyClients(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) const;

    void notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor) const;
    void notifyHandleForces(const std::vector<float> &handleForces) const;
    void notifySettings() const;

    void notifyDragFactor(unsigned short distance, unsigned char dragFactor) const;
    static bool isAnyDeviceConnected();
};
