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
    CscService,
    FtmsService
};

enum class BleSignalStrength : signed char
{
    PowerSaver = -12,
    Default = 0,
    MaxPower = 9,
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
    SetMachineSettings = 21U,
    SetSensorSignalSettings = 22U,
    SetDragFactorSettings = 23U,
    SetStrokeDetectionSettings = 24U,
    RestartDevice = 31U,
    ResponseCode = 32U,
    ResponseCodeFtms = 128U,
};

enum class ResponseOpCodes : unsigned char
{
    Successful = 1U,
    UnsupportedOpCode,
    InvalidParameter,
    OperationFailed,
    ControlNotPermitted
};

enum class OtaResponseOpCodes : unsigned char
{
    Ok,
    NotOk,
    IncorrectFormat,
    IncorrectFirmwareSize,
    ChecksumError,
    InternalStorageError,
};

enum class OtaRequestOpCodes : unsigned char
{
    Begin,
    Package,
    End,
    Abort,
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
    static constexpr unsigned char Other = 0;
    static constexpr unsigned char TopOfShoe = 1;
    static constexpr unsigned char InShoe = 2;
    static constexpr unsigned char Hip = 3;
    static constexpr unsigned char FrontWheel = 4;
    static constexpr unsigned char LeftCrank = 5;
    static constexpr unsigned char RightCrank = 6;
    static constexpr unsigned char LeftPedal = 7;
    static constexpr unsigned char RightPedal = 8;
    static constexpr unsigned char FrontHub = 9;
    static constexpr unsigned char RearDropout = 10;
    static constexpr unsigned char Chainstay = 11;
    static constexpr unsigned char RearWheel = 12;
    static constexpr unsigned char RearHub = 13;
    static constexpr unsigned char Chest = 14;
    static constexpr unsigned char Spider = 15;
    static constexpr unsigned char ChainRing = 16;
};

class CSCFeaturesFlags
{
public:
    static constexpr unsigned char WheelRevolutionDataSupported = (0x01 << 0U);
    static constexpr unsigned char CrankRevolutionDataSupported = (0x01 << 1U);
    static constexpr unsigned char MultipleSensorLocationSupported = (0x01 << 2U);
};

class LogSettingsFlags
{
public:
    static constexpr unsigned char DeltaTimeLoggingNotSupported = (0x00 << 0U);
    static constexpr unsigned char DeltaTimeLoggingDisabled = (0x01 << 0U);
    static constexpr unsigned char DeltaTimeLoggingEnabled = (0x02 << 0U);

    static constexpr unsigned char LogToSdCardNotSupported = (0x00 << 2U);
    static constexpr unsigned char LogToSdCardDisabled = (0x01 << 2U);
    static constexpr unsigned char LogToSdCardEnabled = (0x02 << 2U);

    static constexpr unsigned char LogLevelSilent = (0x00 << 4U);
    static constexpr unsigned char LogLevelFatal = (0x01 << 4U);
    static constexpr unsigned char LogLevelError = (0x02 << 4U);
    static constexpr unsigned char LogLevelWarning = (0x03 << 4U);
    static constexpr unsigned char LogLevelInfo = (0x04 << 4U);
    static constexpr unsigned char LogLevelTrace = (0x05 << 4U);
    static constexpr unsigned char LogLevelVerbose = (0x06 << 4U);
};

class PSCFeaturesFlags
{
public:
    static constexpr unsigned int PedalPowerBalanceSupported = (0x01U << 0U);
    static constexpr unsigned int AccumulatedTorqueSupported = (0x01U << 1U);
    static constexpr unsigned int WheelRevolutionDataSupported = (0x01U << 2U);
    static constexpr unsigned int CrankRevolutionDataSupported = (0x01U << 3U);
    static constexpr unsigned int ExtremeMagnitudesSupported = (0x01U << 4U);
    static constexpr unsigned int ExtremeAnglesSupported = (0x01U << 5U);
    static constexpr unsigned int TopAndBottomDeadSpotAnglesSupported = (0x01U << 6U);
    static constexpr unsigned int AccumulatedEnergySupported = (0x01U << 7U);
    static constexpr unsigned int OffsetCompensationIndicatorSupported = (0x01U << 8U);
    static constexpr unsigned int OffsetCompensationSupported = (0x01U << 9U);
    static constexpr unsigned int CyclingPowerMeasurementCharacteristicContentMaskingSupported = (0x01U << 10U);
    static constexpr unsigned int MultipleSensorLocationsSupported = (0x01U << 11U);
    static constexpr unsigned int CrankLengthAdjustmentSupported = (0x01U << 12U);
    static constexpr unsigned int ChainLengthAdjustmentSupported = (0x01U << 13U);
    static constexpr unsigned int ChainWeightAdjustmentSupported = (0x01U << 14U);
    static constexpr unsigned int SpanLengthAdjustmentSupported = (0x01U << 15U);
    static constexpr unsigned int SensorMeasurementContext = (0x01U << 16U);
    static constexpr unsigned int SensorMeasurementContextForce = (0x00U << 16U);
    static constexpr unsigned int SensorMeasurementContextTorque = (0x01U << 16U);
    static constexpr unsigned int InstantaneousMeasurementDirectionSupported = (0x01U << 17U);
    static constexpr unsigned int FactoryCalibrationDateSupported = (0x01U << 18U);
    static constexpr unsigned int EnhancedOffsetCompensationSupported = (0x01U << 19U);
    static constexpr unsigned int DistributeSystemSupportUnspecified = (0x00U << 20U);
    static constexpr unsigned int DistributeSystemSupportNotinDistributed = (0x01U << 20U);
    static constexpr unsigned int DistributeSystemSupportInDistributed = (0x02U << 20U);
    static constexpr unsigned int DistributeSystemSupportRFU = (0x03U << 20U);
};

class PSCMeasurementFeaturesFlags
{
public:
    static constexpr unsigned short PedalPowerBalancePresent = (0x01U << 0U);
    static constexpr unsigned short PedalPowerBalanceReference = (0x01U << 1U);
    static constexpr unsigned short AccumulatedTorquePresent = (0x01U << 2U);
    static constexpr unsigned short AccumulatedTorqueSource = (0x01U << 3U);
    static constexpr unsigned short AccumulatedTorqueSourceWheel = (0x00U << 3U);
    static constexpr unsigned short AccumulatedTorqueSourceCrank = (0x01U << 3U);
    static constexpr unsigned short WheelRevolutionDataPresent = (0x01U << 4U);
    static constexpr unsigned short CrankRevolutionDataPresent = (0x01U << 5U);
    static constexpr unsigned short ExtremeForceMagnitudesPresent = (0x01U << 6U);
    static constexpr unsigned short ExtremeTorqueMagnitudesPresent = (0x01U << 7U);
    static constexpr unsigned short ExtremeAnglesPresent = (0x01U << 8U);
    static constexpr unsigned short TopDeadSpotAnglePresent = (0x01U << 9U);
    static constexpr unsigned short BottomDeadSpotAnglePresent = (0x01U << 10U);
    static constexpr unsigned short AccumulatedEnergyPresent = (0x01U << 11U);
    static constexpr unsigned short OffsetCompensationIndicator = (0x01U << 12U);
};

class FTMSFeaturesFlags
{
public:
    static constexpr unsigned int AverageSpeedSupported = (0x01U << 0U);
    static constexpr unsigned int CadenceSupported = (0x01U << 1U);
    static constexpr unsigned int TotalDistanceSupported = (0x01U << 2U);
    static constexpr unsigned int InclinationSupported = (0x01U << 3U);
    static constexpr unsigned int ElevationGainSupported = (0x01U << 4U);
    static constexpr unsigned int PaceSupported = (0x01U << 5U);
    static constexpr unsigned int StepCountSupported = (0x01U << 6U);
    static constexpr unsigned int ResistanceLevelSupported = (0x01U << 7U);
    static constexpr unsigned int StrideCountSupported = (0x01U << 8U);
    static constexpr unsigned int ExpendedEnergySupported = (0x01U << 9U);
    static constexpr unsigned int HeartRateMeasurementSupported = (0x01U << 10U);
    static constexpr unsigned int MetabolicEquivalentSupported = (0x01U << 11U);
    static constexpr unsigned int ElapsedTimeSupported = (0x01U << 12U);
    static constexpr unsigned int RemainingTimeSupported = (0x01U << 13U);
    static constexpr unsigned int PowerMeasurementSupported = (0x01U << 14U);
    static constexpr unsigned int ForceOnBeltAndPowerOutputSupported = (0x01U << 15U);
    static constexpr unsigned int UserDataRetentionSupported = (0x01U << 16U);
};

class FTMSMeasurementFeaturesFlags
{
public:
    static constexpr unsigned short MoreDataPresent = (0x01U << 0U);
    static constexpr unsigned short AverageStrokeRatePresent = (0x01U << 1U);
    static constexpr unsigned short TotalDistancePresent = (0x01U << 2U);
    static constexpr unsigned short InstantaneousPacePresent = (0x01U << 3U);
    static constexpr unsigned short AveragePacePresent = (0x01U << 4U);
    static constexpr unsigned short InstantaneousPowerPresent = (0x01U << 5U);
    static constexpr unsigned short AveragePowerPresent = (0x01U << 6U);
    static constexpr unsigned short ResistanceLevelPresent = (0x01U << 7U);
    static constexpr unsigned short ExpendedEnergyPresent = (0x01U << 8U);
    static constexpr unsigned short HeartRatePresent = (0x01U << 9U);
    static constexpr unsigned short MetabolicEquivalentPresent = (0x01U << 10U);
    static constexpr unsigned short ElapsedTimePresent = (0x01U << 11U);
    static constexpr unsigned short RemainingTimePresent = (0x01U << 12U);
};

class FTMSTypeField
{
public:
    static constexpr unsigned short TreadmillSupported = (0x01U << 0U);
    static constexpr unsigned short CrossTrainerSupported = (0x01U << 1U);
    static constexpr unsigned short StepClimberSupported = (0x01U << 2U);
    static constexpr unsigned short StairClimberSupported = (0x01U << 3U);
    static constexpr unsigned short RowerSupported = (0x01U << 4U);
    static constexpr unsigned short IndoorBikeSupported = (0x01U << 5U);
};

class CSCSensorBleFlags
{
public:
    static constexpr unsigned short cscFeaturesFlag = CSCFeaturesFlags::CrankRevolutionDataSupported |
                                                      CSCFeaturesFlags::WheelRevolutionDataSupported;
    static constexpr unsigned char cscMeasurementFeaturesFlag = cscFeaturesFlag;
    static constexpr unsigned short cyclingSpeedCadenceSvcUuid = 0x1816;
    static constexpr unsigned short cscMeasurementUuid = 0x2A5B;
    static constexpr unsigned short cscControlPointUuid = 0x2A55;
    static constexpr unsigned short cscFeatureUuid = 0x2A5C;
    static constexpr unsigned short bleAppearanceCyclingSpeedCadence = 1157;
};

class PSCSensorBleFlags
{
public:
    static constexpr unsigned short pscMeasurementFeaturesFlag =
        PSCMeasurementFeaturesFlags::CrankRevolutionDataPresent |
        PSCMeasurementFeaturesFlags::WheelRevolutionDataPresent;
    static constexpr unsigned int pscFeaturesFlag =
        PSCFeaturesFlags::WheelRevolutionDataSupported |
        PSCFeaturesFlags::CrankRevolutionDataSupported;
    static constexpr unsigned short cyclingPowerSvcUuid = 0x1818;
    static constexpr unsigned short pscMeasurementUuid = 0x2A63;
    static constexpr unsigned short pscControlPointUuid = 0x2A66;
    static constexpr unsigned short pscFeatureUuid = 0x2A65;
    static constexpr unsigned short bleAppearanceCyclingPower = 1156;
};

class FTMSSensorBleFlags
{
public:
    static constexpr unsigned short ftmsMeasurementFeaturesFlag =
        FTMSMeasurementFeaturesFlags::TotalDistancePresent | FTMSMeasurementFeaturesFlags::InstantaneousPacePresent | FTMSMeasurementFeaturesFlags::InstantaneousPowerPresent | FTMSMeasurementFeaturesFlags::ResistanceLevelPresent;
    static constexpr unsigned int ftmsFeaturesFlag =
        FTMSFeaturesFlags::TotalDistanceSupported | FTMSFeaturesFlags::PaceSupported | FTMSFeaturesFlags::ResistanceLevelSupported | FTMSFeaturesFlags::PowerMeasurementSupported;
    static constexpr unsigned short ftmsSvcUuid = 0x1826;
    static constexpr unsigned short ftmsFeaturesUuid = 0x2ACC;
    static constexpr unsigned short rowerDataUuid = 0x2AD1;
    static constexpr unsigned short ftmsControlPointUuid = 0x2AD9;
};

class CommonBleFlags
{
public:
    static constexpr unsigned char sensorLocationFlag = SensorLocations::Other;
    static constexpr unsigned short sensorLocationUuid = 0x2A5D;

    inline static const std::string settingsServiceUuid = "56892de1-7068-4b5a-acaa-473d97b02206";
    inline static const std::string settingsUuid = "54e15528-73b5-4905-9481-89e5184a3364";
    inline static const std::string strokeDetectionSettingsUuid = "5d9c04cd-dcec-4551-8169-8c81f14d9d9d";
    inline static const std::string settingsControlPointUuid = "51ba0a00-8853-477c-bf43-6a09c36aac9f";

    inline static const std::string extendedMetricsServiceUuid = "a72a5762-803b-421d-a759-f0314153da97";
    inline static const std::string extendedMetricsUuid = "808a0d51-efae-4f0c-b2e0-48bc180d65c3";
    inline static const std::string handleForcesUuid = "3d9c2760-cf91-41ee-87e9-fd99d5f129a4";
    inline static const std::string deltaTimesUuid = "ae5d11ea-62f6-4789-b809-6fc93fee92b9";

    inline static const std::string otaServiceUuid = "ed249319-32c3-4e9f-83d7-7bb5aa5d5d4b";
    inline static const std::string otaRxUuid = "fbac1540-698b-40ff-a34e-f39e5b78d1cf";
    inline static const std::string otaTxUuid = "b31126a7-a29b-450a-b0c2-c0516f46b699";

    static constexpr unsigned short batterySvcUuid = 0x180F;
    static constexpr unsigned short batteryLevelUuid = 0x2A19;

    static constexpr unsigned short deviceInfoSvcUuid = 0x180A;
    static constexpr unsigned short modelNumberSvcUuid = 0x2A24;
    static constexpr unsigned short serialNumberSvcUuid = 0x2A25;
    static constexpr unsigned short firmwareNumberSvcUuid = 0x2A26;
    static constexpr unsigned short manufacturerNameSvcUuid = 0x2A29;
};
