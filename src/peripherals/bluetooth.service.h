#pragma once

#include <array>
#include <vector>

#include "NimBLEDevice.h"

#include "../utils/EEPROM.service.interface.h"
#include "../utils/enums.h"
#include "./bluetooth.service.interface.h"
#include "./sd-card.service.interface.h"

using std::vector;

class BluetoothService final : public IBluetoothService
{
    class ControlPointCallbacks final : public NimBLECharacteristicCallbacks
    {
        BluetoothService &bleService;

    public:
        explicit ControlPointCallbacks(BluetoothService &_bleService);

        void onWrite(NimBLECharacteristic *pCharacteristic) override;
    };

    class ChunkedNotifyMetricCallbacks final : public NimBLECharacteristicCallbacks
    {
        BluetoothService &bleService;

    public:
        explicit ChunkedNotifyMetricCallbacks(BluetoothService &_bleService);

        void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue) override;
    };

    class ServerCallbacks final : public NimBLEServerCallbacks
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

    IEEPROMService &eepromService;
    ISdCardService &sdCardService;
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

    static constexpr unsigned char settingsArrayLength = 1U;
    std::array<unsigned char, settingsArrayLength> getSettings() const;

public:
    explicit BluetoothService(IEEPROMService &_eepromService, ISdCardService &_sdCardService);

    void setup() override;
    void startBLEServer() override;
    void stopServer() override;

    void notifyBattery(unsigned char batteryLevel) const override;
    void notifyBaseMetrics(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) override;
    void notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor) override;
    void notifyHandleForces(const std::vector<float> &handleForces) override;
    void notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes) override;
    void notifySettings() const override;

    unsigned short getDeltaTimesMTU() const override;
    bool isDeltaTimesSubscribed() const override;
    bool isAnyDeviceConnected() override;
};
