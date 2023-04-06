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

    static unsigned char constexpr sensorLocationFlag = SensorLocations::Other;

    static unsigned short constexpr cscFeaturesFlag = CSCFeaturesFlags::CrankRevolutionDataSupported |
                                                      CSCFeaturesFlags::WheelRevolutionDataSupported;
    static unsigned char const cscMeasurementFeaturesFlag = cscFeaturesFlag;
    static unsigned short const cyclingSpeedCadenceSvcUuid = 0x1816;
    static unsigned short const cscMeasurementUuid = 0x2A5B;
    static unsigned short const cscControlPointUuid = 0x2A55;
    static unsigned short const cscFeatureUuid = 0x2A5C;
    static unsigned short const bleAppearanceCyclingSpeedCadence = 1157;

    static unsigned short const pscMeasurementFeaturesFlag =
        PSCMeasurementFeaturesFlags::CrankRevolutionDataPresent |
        PSCMeasurementFeaturesFlags::WheelRevolutionDataPresent;
    static unsigned int constexpr pscFeaturesFlag =
        PSCFeaturesFlags::WheelRevolutionDataSupported |
        PSCFeaturesFlags::CrankRevolutionDataSupported;
    static unsigned short const cyclingPowerSvcUuid = 0x1818;
    static unsigned short const pscMeasurementUuid = 0x2A63;
    static unsigned short const pscControlPointUuid = 0x2A66;
    static unsigned short const pscFeatureUuid = 0x2A65;
    static unsigned short const bleAppearanceCyclingPower = 1156;

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
