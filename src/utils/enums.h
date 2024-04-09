#pragma once

#include "NimBLEDevice.h"

enum class StrokeDetectionType : unsigned char
{
    Torque,
    Slope,
    Both
};

enum class CyclePhase : unsigned char
{
    Stopped,
    Recovery,
    Drive
};

enum class BleServiceFlag : unsigned char
{
    CpsService,
    CscService
};

enum class BleSignalStrength : unsigned char
{
    PowerSaver = ESP_PWR_LVL_N12,
    Default = ESP_PWR_LVL_N0,
    MaxPower = ESP_PWR_LVL_P9,
};

enum class ArduinoLogLevel : unsigned char
{
    LogLevelSilent = 0,
    LogLevelFatal = 1,
    LogLevelError = 2,
    LogLevelWarning = 3,
    LogLevelInfo = 4,
    LogLevelNotice = 4,
    LogLevelTrace = 5,
    LogLevelVerbose = 6
};

enum class SettingsOpCodes : unsigned char
{
    SetCumulativeValue = 1U,
    UpdateSensorLocation = 2U,
    RequestSupportedSensorLocations = 3U,
    SetCrankLength = 4U,
    RequestCrankLength = 5U,
    SetChainLength = 6U,
    RequestChainLength = 7U,
    SetChainWeight = 8U,
    RequestChainWeight = 9U,
    SetSpanLength = 10U,
    RequestSpanLength = 11U,
    StartOffsetCompensation = 12U,
    MaskCyclingPowerMeasurementCharacteristicContent = 13U,
    RequestSamplingRate = 14U,
    RequestFactoryCalibrationDate = 15U,
    StartEnhancedOffsetCompensation = 16U,
    SetLogLevel = 17U,
    ChangeBleService = 18U,
    SetDeltaTimeLogging = 19U,
    SetSdCardLogging = 20U,
    ResponseCode = 32U
};

enum class ResponseOpCodes : unsigned char
{
    Successful = 1U,
    UnsupportedOpCode,
    InvalidParameter,
    OperationFailed,
};

enum class BaudRates : unsigned int
{
    Baud9600 = 9'600U,
    Baud14400 = 14'400U,
    Baud19200 = 19'200U,
    Baud33600 = 33'600U,
    Baud38400 = 38'400U,
    Baud56000 = 56'000U,
    Baud57600 = 57'600U,
    Baud76800 = 76'800U,
    Baud115200 = 115'200U,
    Baud128000 = 128'000U,
    Baud153600 = 153'600U,
    Baud230400 = 230'400U,
    Baud460800 = 460'800U,
    Baud921600 = 921'600U,
    Baud1500000 = 1'500'000U,
};

class SensorLocations
{
public:
    static const unsigned char Other = 0;
    static const unsigned char TopOfShoe = 1;
    static const unsigned char InShoe = 2;
    static const unsigned char Hip = 3;
    static const unsigned char FrontWheel = 4;
    static const unsigned char LeftCrank = 5;
    static const unsigned char RightCrank = 6;
    static const unsigned char LeftPedal = 7;
    static const unsigned char RightPedal = 8;
    static const unsigned char FrontHub = 9;
    static const unsigned char RearDropout = 10;
    static const unsigned char Chainstay = 11;
    static const unsigned char RearWheel = 12;
    static const unsigned char RearHub = 13;
    static const unsigned char Chest = 14;
    static const unsigned char Spider = 15;
    static const unsigned char ChainRing = 16;
};

class CSCFeaturesFlags
{
public:
    static const unsigned char WheelRevolutionDataSupported = (0x01 << 0U);
    static const unsigned char CrankRevolutionDataSupported = (0x01 << 1U);
    static const unsigned char MultipleSensorLocationSupported = (0x01 << 2U);
};

class LogSettingsFlags
{
public:
    static const unsigned char DeltaTimeLoggingNotSupported = (0x00 << 0U);
    static const unsigned char DeltaTimeLoggingDisabled = (0x01 << 0U);
    static const unsigned char DeltaTimeLoggingEnabled = (0x02 << 0U);

    static const unsigned char LogToSdCardNotSupported = (0x00 << 2U);
    static const unsigned char LogToSdCardDisabled = (0x01 << 2U);
    static const unsigned char LogToSdCardEnabled = (0x02 << 2U);

    static const unsigned char LogLevelSilent = (0x00 << 4U);
    static const unsigned char LogLevelFatal = (0x01 << 4U);
    static const unsigned char LogLevelError = (0x02 << 4U);
    static const unsigned char LogLevelWarning = (0x03 << 4U);
    static const unsigned char LogLevelInfo = (0x04 << 4U);
    static const unsigned char LogLevelTrace = (0x05 << 4U);
    static const unsigned char LogLevelVerbose = (0x06 << 4U);
};

class PSCFeaturesFlags
{
public:
    static const unsigned int PedalPowerBalanceSupported = (0x01U << 0U);
    static const unsigned int AccumulatedTorqueSupported = (0x01U << 1U);
    static const unsigned int WheelRevolutionDataSupported = (0x01U << 2U);
    static const unsigned int CrankRevolutionDataSupported = (0x01U << 3U);
    static const unsigned int ExtremeMagnitudesSupported = (0x01U << 4U);
    static const unsigned int ExtremeAnglesSupported = (0x01U << 5U);
    static const unsigned int TopAndBottomDeadSpotAnglesSupported = (0x01U << 6U);
    static const unsigned int AccumulatedEnergySupported = (0x01U << 7U);
    static const unsigned int OffsetCompensationIndicatorSupported = (0x01U << 8U);
    static const unsigned int OffsetCompensationSupported = (0x01U << 9U);
    static const unsigned int CyclingPowerMeasurementCharacteristicContentMaskingSupported = (0x01U << 10U);
    static const unsigned int MultipleSensorLocationsSupported = (0x01U << 11U);
    static const unsigned int CrankLengthAdjustmentSupported = (0x01U << 12U);
    static const unsigned int ChainLengthAdjustmentSupported = (0x01U << 13U);
    static const unsigned int ChainWeightAdjustmentSupported = (0x01U << 14U);
    static const unsigned int SpanLengthAdjustmentSupported = (0x01U << 15U);
    static const unsigned int SensorMeasurementContext = (0x01U << 16U);
    static const unsigned int SensorMeasurementContextForce = (0x00U << 16U);
    static const unsigned int SensorMeasurementContextTorque = (0x01U << 16U);
    static const unsigned int InstantaneousMeasurementDirectionSupported = (0x01U << 17U);
    static const unsigned int FactoryCalibrationDateSupported = (0x01U << 18U);
    static const unsigned int EnhancedOffsetCompensationSupported = (0x01U << 19U);
    static const unsigned int DistributeSystemSupportUnspecified = (0x00U << 20U);
    static const unsigned int DistributeSystemSupportNotinDistributed = (0x01U << 20U);
    static const unsigned int DistributeSystemSupportInDistributed = (0x02U << 20U);
    static const unsigned int DistributeSystemSupportRFU = (0x03U << 20U);
};

class PSCMeasurementFeaturesFlags
{
public:
    static const unsigned short PedalPowerBalancePresent = (0x01U << 0U);
    static const unsigned short PedalPowerBalanceReference = (0x01U << 1U);
    static const unsigned short AccumulatedTorquePresent = (0x01U << 2U);
    static const unsigned short AccumulatedTorqueSource = (0x01U << 3U);
    static const unsigned short AccumulatedTorqueSourceWheel = (0x00U << 3U);
    static const unsigned short AccumulatedTorqueSourceCrank = (0x01U << 3U);
    static const unsigned short WheelRevolutionDataPresent = (0x01U << 4U);
    static const unsigned short CrankRevolutionDataPresent = (0x01U << 5U);
    static const unsigned short ExtremeForceMagnitudesPresent = (0x01U << 6U);
    static const unsigned short ExtremeTorqueMagnitudesPresent = (0x01U << 7U);
    static const unsigned short ExtremeAnglesPresent = (0x01U << 8U);
    static const unsigned short TopDeadSpotAnglePresent = (0x01U << 9U);
    static const unsigned short BottomDeadSpotAnglePresent = (0x01U << 10U);
    static const unsigned short AccumulatedEnergyPresent = (0x01U << 11U);
    static const unsigned short OffsetCompensationIndicator = (0x01U << 12U);
};

class CSCSensorBleFlags
{
public:
    static constexpr unsigned short cscFeaturesFlag = CSCFeaturesFlags::CrankRevolutionDataSupported |
                                                      CSCFeaturesFlags::WheelRevolutionDataSupported;
    static const unsigned char cscMeasurementFeaturesFlag = cscFeaturesFlag;
    static const unsigned short cyclingSpeedCadenceSvcUuid = 0x1816;
    static const unsigned short cscMeasurementUuid = 0x2A5B;
    static const unsigned short cscControlPointUuid = 0x2A55;
    static const unsigned short cscFeatureUuid = 0x2A5C;
    static const unsigned short bleAppearanceCyclingSpeedCadence = 1157;
};

class PSCSensorBleFlags
{
public:
    static const unsigned short pscMeasurementFeaturesFlag =
        PSCMeasurementFeaturesFlags::CrankRevolutionDataPresent |
        PSCMeasurementFeaturesFlags::WheelRevolutionDataPresent;
    static constexpr unsigned int pscFeaturesFlag =
        PSCFeaturesFlags::WheelRevolutionDataSupported |
        PSCFeaturesFlags::CrankRevolutionDataSupported;
    static const unsigned short cyclingPowerSvcUuid = 0x1818;
    static const unsigned short pscMeasurementUuid = 0x2A63;
    static const unsigned short pscControlPointUuid = 0x2A66;
    static const unsigned short pscFeatureUuid = 0x2A65;
    static const unsigned short bleAppearanceCyclingPower = 1156;
};

class CommonBleFlags
{
public:
    static constexpr unsigned char sensorLocationFlag = SensorLocations::Other;

    static const unsigned short sensorLocationUuid = 0x2A5D;
    inline static const std::string dragFactorUuid = "CE060031-43E5-11E4-916C-0800200C9A66";
    inline static const std::string dragFactorSvcUuid = "CE060030-43E5-11E4-916C-0800200C9A66";

    inline static const std::string settingsServiceUuid = "56892de1-7068-4b5a-acaa-473d97b02206";
    inline static const std::string settingsUuid = "54e15528-73b5-4905-9481-89e5184a3364";
    inline static const std::string settingsControlPointUuid = "51ba0a00-8853-477c-bf43-6a09c36aac9f";

    inline static const std::string extendedMetricsServiceUuid = "a72a5762-803b-421d-a759-f0314153da97";
    inline static const std::string extendedMetricsUuid = "808a0d51-efae-4f0c-b2e0-48bc180d65c3";
    inline static const std::string handleForcesUuid = "3d9c2760-cf91-41ee-87e9-fd99d5f129a4";
    inline static const std::string deltaTimesUuid = "ae5d11ea-62f6-4789-b809-6fc93fee92b9";

    static const unsigned short batterySvcUuid = 0x180F;
    static const unsigned short batteryLevelUuid = 0x2A19;

    static const unsigned short deviceInfoSvcUuid = 0x180A;
    static const unsigned short modelNumberSvcUuid = 0x2A24;
    static const unsigned short serialNumberSvcUuid = 0x2A25;
    static const unsigned short softwareNumberSvcUuid = 0x2A28;
    static const unsigned short manufacturerNameSvcUuid = 0x2A29;
};
