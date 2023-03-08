#pragma once

enum class CyclePhase
{
    Stopped,
    Recovery,
    Drive
};

enum class BleServiceFlag
{
    CpsService,
    CscService
};

enum class ArduinoLogLevel
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

enum class PSCOpCodes
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
    ResponseCode = 32U
};

enum class PSCResponseOpCodes
{
    Successful = 1U,
    UnsupportedOpCode,
    InvalidParameter,
    OperationFailed,
};

class SensorLocations
{
public:
    static unsigned char const Other = 0;
    static unsigned char const TopOfShoe = 1;
    static unsigned char const InShoe = 2;
    static unsigned char const Hip = 3;
    static unsigned char const FrontWheel = 4;
    static unsigned char const LeftCrank = 5;
    static unsigned char const RightCrank = 6;
    static unsigned char const LeftPedal = 7;
    static unsigned char const RightPedal = 8;
    static unsigned char const FrontHub = 9;
    static unsigned char const RearDropout = 10;
    static unsigned char const Chainstay = 11;
    static unsigned char const RearWheel = 12;
    static unsigned char const RearHub = 13;
    static unsigned char const Chest = 14;
    static unsigned char const Spider = 15;
    static unsigned char const ChainRing = 16;
};

class CSCFeaturesFlags
{
public:
    static unsigned char const WheelRevolutionDataSupported = (0x01 << 0U);
    static unsigned char const CrankRevolutionDataSupported = (0x01 << 1U);
    static unsigned char const MultipleSensorLocationSupported = (0x01 << 2U);
};

class PSCFeaturesFlags
{
public:
    static unsigned int const PedalPowerBalanceSupported = (0x01U << 0U);
    static unsigned int const AccumulatedTorqueSupported = (0x01U << 1U);
    static unsigned int const WheelRevolutionDataSupported = (0x01U << 2U);
    static unsigned int const CrankRevolutionDataSupported = (0x01U << 3U);
    static unsigned int const ExtremeMagnitudesSupported = (0x01U << 4U);
    static unsigned int const ExtremeAnglesSupported = (0x01U << 5U);
    static unsigned int const TopAndBottomDeadSpotAnglesSupported = (0x01U << 6U);
    static unsigned int const AccumulatedEnergySupported = (0x01U << 7U);
    static unsigned int const OffsetCompensationIndicatorSupported = (0x01U << 8U);
    static unsigned int const OffsetCompensationSupported = (0x01U << 9U);
    static unsigned int const CyclingPowerMeasurementCharacteristicContentMaskingSupported = (0x01U << 10U);
    static unsigned int const MultipleSensorLocationsSupported = (0x01U << 11U);
    static unsigned int const CrankLengthAdjustmentSupported = (0x01U << 12U);
    static unsigned int const ChainLengthAdjustmentSupported = (0x01U << 13U);
    static unsigned int const ChainWeightAdjustmentSupported = (0x01U << 14U);
    static unsigned int const SpanLengthAdjustmentSupported = (0x01U << 15U);
    static unsigned int const SensorMeasurementContext = (0x01U << 16U);
    static unsigned int const SensorMeasurementContextForce = (0x00U << 16U);
    static unsigned int const SensorMeasurementContextTorque = (0x01U << 16U);
    static unsigned int const InstantaneousMeasurementDirectionSupported = (0x01U << 17U);
    static unsigned int const FactoryCalibrationDateSupported = (0x01U << 18U);
    static unsigned int const EnhancedOffsetCompensationSupported = (0x01U << 19U);
    static unsigned int const DistributeSystemSupportUnspecified = (0x00U << 20U);
    static unsigned int const DistributeSystemSupportNotinDistributed = (0x01U << 20U);
    static unsigned int const DistributeSystemSupportInDistributed = (0x02U << 20U);
    static unsigned int const DistributeSystemSupportRFU = (0x03U << 20U);
};

class PSCMeasurementFeaturesFlags
{
public:
    static unsigned short const PedalPowerBalancePresent = (0x01U << 0U);
    static unsigned short const PedalPowerBalanceReference = (0x01U << 1U);
    static unsigned short const AccumulatedTorquePresent = (0x01U << 2U);
    static unsigned short const AccumulatedTorqueSource = (0x01U << 3U);
    static unsigned short const AccumulatedTorqueSourceWheel = (0x00U << 3U);
    static unsigned short const AccumulatedTorqueSourceCrank = (0x01U << 3U);
    static unsigned short const WheelRevolutionDataPresent = (0x01U << 4U);
    static unsigned short const CrankRevolutionDataPresent = (0x01U << 5U);
    static unsigned short const ExtremeForceMagnitudesPresent = (0x01U << 6U);
    static unsigned short const ExtremeTorqueMagnitudesPresent = (0x01U << 7U);
    static unsigned short const ExtremeAnglesPresent = (0x01U << 8U);
    static unsigned short const TopDeadSpotAnglePresent = (0x01U << 9U);
    static unsigned short const BottomDeadSpotAnglePresent = (0x01U << 10U);
    static unsigned short const AccumulatedEnergyPresent = (0x01U << 11U);
    static unsigned short const OffsetCompensationIndicator = (0x01U << 12U);
};