#pragma once

#include <array>

#include "NimBLEDevice.h"

class BluetoothService
{
    enum SensorLocations
    {
        Other = 0x00,
        TopOfShoe = 0x01,
        InShoe = 0x02,
        Hip = 0x03,
        FrontWheel = 0x04,
        LeftCrank = 0x05,
        RightCrank = 0x06,
        LeftPedal = 0x07,
        RightPedal = 0x08,
        FrontHub = 0x09,
        RearDropout = 0x010,
        Chainstay = 0x011,
        RearWheel = 0x012,
        RearHub = 0x013,
        Chest = 0x014,
        Spider = 0x015,
        ChainRing = 0x01u
    };

    enum PSCFeaturesFlags
    {
        PedalPowerBalanceSupported = (0x01u << 0u),
        AccumulatedTorqueSupported = (0x01u << 1u),
        WheelRevolutionDataSupported = (0x01u << 2u),
        CrankRevolutionDataSupported = (0x01u << 3u),
        ExtremeMagnitudesSupported = (0x01u << 4u),
        ExtremeAnglesSupported = (0x01u << 5u),
        TopAndBottomDeadSpotAnglesSupported = (0x01u << 6u),
        AccumulatedEnergySupported = (0x01u << 7u),
        OffsetCompensationIndicatorSupported = (0x01u << 8u),
        OffsetCompensationSupported = (0x01u << 9u),
        CyclingPowerMeasurementCharacteristicContentMaskingSupported = (0x01u << 10u),
        MultipleSensorLocationsSupported = (0x01u << 11u),
        CrankLengthAdjustmentSupported = (0x01u << 12u),
        ChainLengthAdjustmentSupported = (0x01u << 13u),
        ChainWeightAdjustmentSupported = (0x01u << 14u),
        SpanLengthAdjustmentSupported = (0x01u << 15u),
        SensorMeasurementContext = (0x01u << 16u),
        SensorMeasurementContextForce = (0x00u << 16u),
        SensorMeasurementContextTorque = (0x01u << 16u),
        InstantaneousMeasurementDirectionSupported = (0x01u << 17u),
        FactoryCalibrationDateSupported = (0x01u << 18u),
        EnhancedOffsetCompensationSupported = (0x01u << 19u)
    };

    enum PSCMeasurementFeaturesFlags
    {
        PedalPowerBalancePresent = (0x01u << 0u),
        PedalPowerBalanceReference = (0x01u << 1u),
        AccumulatedTorquePresent = (0x01u << 2u),
        AccumulatedTorqueSource = (0x01u << 3u),
        AccumulatedTorqueSourceWheel = (0x00u << 3u),
        AccumulatedTorqueSourceCrank = (0x01u << 3u),
        WheelRevolutionDataPresent = (0x01u << 4u),
        CrankRevolutionDataPresent = (0x01u << 5u),
        ExtremeForceMagnitudesPresent = (0x01u << 6u),
        ExtremeTorqueMagnitudesPresent = (0x01u << 7u),
        ExtremeAnglesPresent = (0x01u << 8u),
        TopDeadSpotAnglePresent = (0x01u << 9u),
        BottomDeadSpotAnglePresent = (0x01u << 10u),
        AccumulatedEnergyPresent = (0x01u << 11u),
        OffsetCompensationIndicator = (0x01u << 12u)
    };

    static byte constexpr sensorLocationFlag = SensorLocations::Other;

    static byte const cscMeasurementFeaturesFlag = 0b11;
    static unsigned short constexpr cscFeaturesFlag = 0b11;
    static unsigned short const cyclingSpeedCadenceSvcUuid = 0x1816;
    static unsigned short const cscMeasurementUuid = 0x2A5B;
    static unsigned short const cscControlPointUuid = 0x2A55;
    static unsigned short const cscFeatureUuid = 0x2A5C;

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

    byte ledState = HIGH;

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
    void notifyCsc(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount) const;
    void notifyPsc(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) const;
    void notifyDragFactor(unsigned short distance, byte dragFactor) const;
    bool isAnyDeviceConnected() const;
    void updateLed();
};
