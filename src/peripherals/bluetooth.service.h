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

    class ChunkedNotifyMetricCallbacks : public NimBLECharacteristicCallbacks
    {
        BluetoothService &bleService;

    public:
        explicit ChunkedNotifyMetricCallbacks(BluetoothService &_bleService);

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

    struct DeltaTimesParameters
    {
        NimBLECharacteristic *characteristic = nullptr;
        vector<unsigned long> deltaTimes;
        vector<unsigned char> clientIds;

        static void task(void *parameters);
    } deltaTimesParameters;

    struct HandleForcesParameters
    {
        NimBLECharacteristic *characteristic = nullptr;
        unsigned short mtu = 512U;
        vector<float> handleForces;
        vector<unsigned char> clientIds;

        static void task(void *parameters);
    } handleForcesParameters;

    struct ExtendedMetricParameters
    {
        NimBLECharacteristic *characteristic = nullptr;
        short avgStrokePower = 0;
        unsigned short recoveryDuration = 0;
        unsigned short driveDuration = 0;
        unsigned char dragFactor = 0;

        static void task(void *parameters);
    } extendedMetricsParameters;

    struct BaseMetricsParameters
    {
        NimBLECharacteristic *characteristic = nullptr;
        unsigned short revTime = 0;
        unsigned int revCount = 0;
        unsigned short strokeTime = 0;
        unsigned short strokeCount = 0;
        short avgStrokePower = 0;

        static void pscTask(void *parameters);
        static void cscTask(void *parameters);
    } baseMetricsParameters;

    EEPROMService &eepromService;
    SdCardService &sdCardService;
    ControlPointCallbacks controlPointCallbacks;
    ChunkedNotifyMetricCallbacks chunkedNotifyMetricCallbacks;
    ServerCallbacks serverCallbacks;

    NimBLECharacteristic *batteryLevelCharacteristic = nullptr;
    NimBLECharacteristic *settingsCharacteristic = nullptr;

    void setupBleDevice();
    void setupServices();
    void setupAdvertisement() const;

    NimBLEService *setupCscServices(NimBLEServer *server);
    NimBLEService *setupPscServices(NimBLEServer *server);
    NimBLEService *setupExtendedMetricsServices(NimBLEServer *server);
    NimBLEService *setupSettingsServices(NimBLEServer *server);
    static NimBLEService *setupDeviceInfoServices(NimBLEServer *server);

    static const unsigned char settingsArrayLength = 1U;
    std::array<unsigned char, settingsArrayLength> getSettings() const;

public:
    explicit BluetoothService(EEPROMService &_eepromService, SdCardService &_sdCardService);

    void setup();
    static void startBLEServer();
    static void stopServer();

    void notifyBattery(unsigned char batteryLevel) const;
    void notifyBaseMetrics(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower);
    void notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor);
    void notifyHandleForces(const std::vector<float> &handleForces);
    void notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes);
    void notifySettings() const;

    unsigned short getDeltaTimesMTU() const;
    bool isDeltaTimesSubscribed() const;
    static bool isAnyDeviceConnected();
};
