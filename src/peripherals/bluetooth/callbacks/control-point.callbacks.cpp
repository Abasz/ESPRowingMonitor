#include <array>
#include <cstring>
#include <iterator>
#include <string>
#include <utility>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "globals.h"

#include "./control-point.callbacks.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../../utils/configuration.h"
#include "../../../utils/enums.h"
#include "../../../utils/settings.model.h"
#include "../ble-services/settings.service.interface.h"
#include "../ble.enums.h"

using std::array;

namespace
{

    [[nodiscard]] auto makeResponse(
        const SettingsOpCodes opCode,
        const unsigned char echoedOpCode,
        const ResponseOpCodes result) noexcept -> array<unsigned char, 3U>
    {
        return {
            std::to_underlying(opCode),
            echoedOpCode,
            std::to_underlying(result),
        };
    }

}

ControlPointCallbacks::ControlPointCallbacks(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService) : settingsBleService(_settingsBleService), eepromService(_eepromService)
{
}

void ControlPointCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic, [[maybe_unused]] NimBLEConnInfo &connInfo)
{
    NimBLEAttValue message = pCharacteristic->getValue();

    Log.verboseln("Incoming connection");

    if (message.size() == 0)
    {
        Log.infoln("Invalid request, no Op Code");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, 0U, ResponseOpCodes::OperationFailed));
        pCharacteristic->indicate();

        return;
    }

    Log.infoln("Op Code: %d; Length: %d", message[0], message.size());

    const auto isFtms = eepromService.getBleServiceFlag() == BleServiceFlag::FtmsService;

    switch (message[0])
    {

    // The case values below are CPS (Cycling Power Service) op code names reused as numeric identifiers for FTMS.
    case std::to_underlying(SettingsOpCodes::RequestControlFtms):
    case std::to_underlying(SettingsOpCodes::SetCumulativeValue): // FTMS Reset
    case std::to_underlying(SettingsOpCodes::RequestChainLength): // FTMS Start or Resume
    case std::to_underlying(SettingsOpCodes::SetChainWeight):     // FTMS Stop or Pause
    {
        Log.infoln("%s Op Code: %d", isFtms ? "Control Not Permitted" : "Not Supported", message[0]);

        pCharacteristic->setValue(makeResponse(
            isFtms ? SettingsOpCodes::ResponseCodeFtms : SettingsOpCodes::ResponseCode,
            message[0],
            isFtms ? ResponseOpCodes::ControlNotPermitted : ResponseOpCodes::UnsupportedOpCode));
    }
    break;

    case std::to_underlying(SettingsOpCodes::SetLogLevel):
    {
        Log.infoln("Set LogLevel");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], processLogLevel(message)));
    }
    break;

    case std::to_underlying(SettingsOpCodes::ChangeBleService):
    {
        Log.infoln("Change BLE Service");

        if (message.size() != 2 || !isInBounds(static_cast<unsigned int>(message[1]), 0U, 2U))
        {
            pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], ResponseOpCodes::InvalidParameter));

            break;
        }

        processBleServiceChange(message, pCharacteristic);

        return;
    }

    case std::to_underlying(SettingsOpCodes::SetSdCardLogging):
    {
        Log.infoln("Change Sd Card Logging");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], processSdCardLogging(message)));
    }
    break;

    case std::to_underlying(SettingsOpCodes::SetDeltaTimeLogging):
    {
        Log.infoln("Change deltaTime logging");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], processDeltaTimeLogging(message)));
    }
    break;

    case std::to_underlying(SettingsOpCodes::SetMachineSettings):
    {
        Log.infoln("Change Machine Settings");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], processMachineSettingsChange(message)));
    }
    break;

    case std::to_underlying(SettingsOpCodes::SetSensorSignalSettings):
    {
        Log.infoln("Change Sensor Signal Filter Settings");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], processSensorSignalSettingsChange(message)));
    }
    break;

    case std::to_underlying(SettingsOpCodes::SetDragFactorSettings):
    {
        Log.infoln("Change Drag Factor Settings");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], processDragFactorSettingsChange(message)));
    }
    break;

    case std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings):
    {
        Log.infoln("Change Stroke Detection Settings");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], processStrokeDetectionSettingsChange(message)));
    }
    break;

    case std::to_underlying(SettingsOpCodes::RestartDevice):
    {
        Log.verboseln("Restarting device...");
        pCharacteristic->setValue(makeResponse(SettingsOpCodes::ResponseCode, message[0], ResponseOpCodes::Successful));
        restartWithDelay(100);
    }
    break;

    default:
    {
        Log.infoln("Not Supported Op Code: %d", message[0]);
        pCharacteristic->setValue(makeResponse(
            isFtms ? SettingsOpCodes::ResponseCodeFtms : SettingsOpCodes::ResponseCode,
            message[0],
            ResponseOpCodes::UnsupportedOpCode));
    }
    break;
    }

    Log.verboseln("Send indicate");
    pCharacteristic->indicate();
}

ResponseOpCodes ControlPointCallbacks::processLogLevel(const NimBLEAttValue &message)
{
    if (message.size() != 2 || !isInBounds(static_cast<unsigned int>(message[1]), 0U, 6U))
    {
        return ResponseOpCodes::InvalidParameter;
    }

    Log.infoln("New LogLevel: %d", message[1]);
    eepromService.setLogLevel(ArduinoLogLevel{message[1]});

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processSdCardLogging(const NimBLEAttValue &message)
{
    if (message.size() != 2 || !isInBounds(static_cast<unsigned int>(message[1]), 0U, 1U))
    {
        Log.infoln("Invalid OP command for setting SD Card deltaTime logging, this should be a bool: %d", message[1]);

        return ResponseOpCodes::InvalidParameter;
    }

    const auto shouldEnable = static_cast<bool>(message[1]);
    Log.infoln("%s SdCard logging", shouldEnable ? "Enable" : "Disable");
    eepromService.setLogToSdCard(shouldEnable);

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processDeltaTimeLogging(const NimBLEAttValue &message)
{
    if (message.size() != 2 || !isInBounds(static_cast<unsigned int>(message[1]), 0U, 1U))
    {
        Log.infoln("Invalid OP command for setting deltaTime logging, this should be a bool: %d", message[1]);

        return ResponseOpCodes::InvalidParameter;
    }

    const auto shouldEnable = static_cast<bool>(message[1]);

    Log.infoln("%s deltaTime logging", shouldEnable ? "Enable" : "Disable");
    eepromService.setLogToBluetooth(shouldEnable);

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

void ControlPointCallbacks::processBleServiceChange(const NimBLEAttValue &message, NimBLECharacteristic *const pCharacteristic)
{
    std::string flagString;
    switch (BleServiceFlag{message[1]})
    {
    case BleServiceFlag::CpsService:
        flagString = "CPS";
        break;

    case BleServiceFlag::CscService:
        flagString = "CSC";
        break;

    case BleServiceFlag::FtmsService:
        flagString = "FTMS";
        break;
    }

    Log.infoln("New BLE Service: %s", flagString.c_str());
    eepromService.setBleServiceFlag(BleServiceFlag{message[1]});
    array<unsigned char, 3U> temp = {
        std::to_underlying(SettingsOpCodes::ResponseCode),
        message[0],
        std::to_underlying(ResponseOpCodes::Successful)};
    pCharacteristic->setValue(temp);
    pCharacteristic->indicate();

    settingsBleService.broadcastSettings();

    Log.verboseln("Restarting device...");
    restartWithDelay(100);
}

ResponseOpCodes ControlPointCallbacks::processMachineSettingsChange(const NimBLEAttValue &message)
{
    if constexpr (!Configurations::isRuntimeSettingsEnabled)
    {
        return ResponseOpCodes::UnsupportedOpCode;
    }

    const auto opCodePayloadSize = 1U;
    if (message.size() != opCodePayloadSize + ISettingsBleService::machineSettingsPayloadSize)
    {
        Log.infoln("Malformed OP command for changing machine settings");

        return ResponseOpCodes::InvalidParameter;
    }
    auto bytePosition = opCodePayloadSize;

    float flywheelInertia = 0.0F;
    std::memcpy(&flywheelInertia, std::next(message.data(), bytePosition), ISettingsBleService::flywheelInertiaPayloadSize);
    bytePosition += ISettingsBleService::flywheelInertiaPayloadSize;

    const float magicNumber = static_cast<float>(message[bytePosition]) / ISettingsBleService::magicNumberScale;
    bytePosition += ISettingsBleService::magicNumberPayloadSize;

    unsigned char impulsesPerRevolution = message[bytePosition];
    bytePosition += ISettingsBleService::impulsesPerRevolutionPayloadSize;

    const auto mToCm = 100.0F;
    const float sprocketRadius = static_cast<float>(message[bytePosition] | message[bytePosition + 1] << 8) /
                                 ISettingsBleService::sprocketRadiusScale / mToCm;

    const RowerProfile::MachineSettings newMachineSettings{
        .impulsesPerRevolution = impulsesPerRevolution,
        .flywheelInertia = flywheelInertia,
        .concept2MagicNumber = magicNumber,
        .sprocketRadius = sprocketRadius,
    };

    if (!eepromService.validateMachineSettings(newMachineSettings))
    {
        return ResponseOpCodes::OperationFailed;
    }

    eepromService.setMachineSettings(newMachineSettings);

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processSensorSignalSettingsChange(const NimBLEAttValue &message)
{
    if constexpr (!Configurations::isRuntimeSettingsEnabled)
    {
        return ResponseOpCodes::UnsupportedOpCode;
    }

    const auto opCodePayloadSize = 1U;
    if (message.size() != opCodePayloadSize + ISettingsBleService::sensorSignalSettingsPayloadSize)
    {
        Log.infoln("Malformed OP command for sensor signal settings");

        return ResponseOpCodes::InvalidParameter;
    }
    auto bytePosition = opCodePayloadSize;

    const unsigned short rotationDebounce = message[bytePosition] * static_cast<unsigned short>(ISettingsBleService::debounceTimeScale);
    bytePosition += ISettingsBleService::rotationDebouncePayloadSize;

    const auto rowingStoppedThresholdPeriod = message[bytePosition] * static_cast<unsigned int>(ISettingsBleService::rowingStoppedThresholdScale);

    const RowerProfile::SensorSignalSettings newSensorSignalSettings{
        .rotationDebounceTimeMin = rotationDebounce,
        .rowingStoppedThresholdPeriod = rowingStoppedThresholdPeriod,
    };

    if (!eepromService.validateSensorSignalSettings(newSensorSignalSettings))
    {
        return ResponseOpCodes::OperationFailed;
    }

    eepromService.setSensorSignalSettings(newSensorSignalSettings);

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processDragFactorSettingsChange(const NimBLEAttValue &message)
{
    if constexpr (!Configurations::isRuntimeSettingsEnabled)
    {
        return ResponseOpCodes::UnsupportedOpCode;
    }

    const auto opCodePayloadSize = 1U;
    if (message.size() != opCodePayloadSize + ISettingsBleService::dragFactorSettingsPayloadSize)
    {
        Log.infoln("Malformed OP command for drag factor settings");

        return ResponseOpCodes::InvalidParameter;
    }
    auto bytePosition = opCodePayloadSize;

    const auto goodnessOfFitThreshold = static_cast<float>(message[bytePosition]) / ISettingsBleService::goodnessOfFitThresholdScale;
    bytePosition += ISettingsBleService::goodnessOfFitPayloadSize;

    const auto dragFactorRecoveryPeriod = message[bytePosition] * static_cast<unsigned int>(ISettingsBleService::dragFactorRecoveryPeriodScale);
    bytePosition += ISettingsBleService::dragFactorRecoveryPeriodPayloadSize;

    const float lowerDragFactorThreshold = static_cast<float>(message[bytePosition] | message[bytePosition + 1] << 8) / ISettingsBleService::dragFactorThresholdScale;
    bytePosition += ISettingsBleService::lowerDragFactorPayloadSize;

    const float upperDragFactorThreshold = static_cast<float>(message[bytePosition] | message[bytePosition + 1] << 8) / ISettingsBleService::dragFactorThresholdScale;
    bytePosition += ISettingsBleService::upperDragFactorPayloadSize;

    const unsigned char dragCoefficientsArrayLength = message[bytePosition];

    const RowerProfile::DragFactorSettings newDragFactorSettings{
        .goodnessOfFitThreshold = goodnessOfFitThreshold,
        .maxDragFactorRecoveryPeriod = dragFactorRecoveryPeriod,
        .lowerDragFactorThreshold = lowerDragFactorThreshold,
        .upperDragFactorThreshold = upperDragFactorThreshold,
        .dragCoefficientsArrayLength = dragCoefficientsArrayLength,
    };

    if (!eepromService.validateDragFactorSettings(newDragFactorSettings))
    {
        return ResponseOpCodes::OperationFailed;
    }

    eepromService.setDragFactorSettings(newDragFactorSettings);

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processStrokeDetectionSettingsChange(const NimBLEAttValue &message)
{
    if constexpr (!Configurations::isRuntimeSettingsEnabled)
    {
        return ResponseOpCodes::UnsupportedOpCode;
    }

    const auto opCodePayloadSize = 1U;
    if (message.size() != opCodePayloadSize + ISettingsBleService::strokeSettingsPayloadSize)
    {
        Log.infoln("Malformed OP command for stroke detection settings");

        return ResponseOpCodes::InvalidParameter;
    }
    auto bytePosition = opCodePayloadSize;

    const unsigned char impulseAndDetection = message[bytePosition];
    const auto strokeDetectionType = static_cast<StrokeDetectionType>(impulseAndDetection & 0x03);
    const unsigned char impulseDataArrayLength = (impulseAndDetection >> 2U) & 0x1F;
    bytePosition += ISettingsBleService::impulseAndDetectionTypePayloadSize;

    const float minimumPoweredTorque = static_cast<float>(static_cast<short>(message[bytePosition] | message[bytePosition + 1] << 8)) / ISettingsBleService::poweredTorqueScale;
    bytePosition += ISettingsBleService::poweredTorquePayloadSize;

    const float minimumDragTorque = static_cast<float>(static_cast<short>(message[bytePosition] | message[bytePosition + 1] << 8)) / ISettingsBleService::dragTorqueScale;
    bytePosition += ISettingsBleService::dragTorquePayloadSize;

    const float minimumRecoverySlope = static_cast<float>(static_cast<short>(message[bytePosition] | message[bytePosition + 1] << 8)) / ISettingsBleService::recoverySlopeScale;
    bytePosition += ISettingsBleService::recoverySlopePayloadSize;

    const auto strokeTimes = static_cast<unsigned int>(message[bytePosition] | message[bytePosition + 1] << 8 | message[bytePosition + 2] << 16);
    const auto minimumRecoveryTime = (strokeTimes & 0xFFF) * ISettingsBleService::minimumStrokeTimesScale;
    const auto minimumDriveTime = (strokeTimes >> 12) * ISettingsBleService::minimumStrokeTimesScale;
    bytePosition += ISettingsBleService::minimumStrokeTimesPayloadSize;

    const auto driveHandleForcesMaxCapacity = message[bytePosition];

    const RowerProfile::StrokePhaseDetectionSettings newStrokeDetectionSettings{
        .strokeDetectionType = strokeDetectionType,
        .minimumPoweredTorque = minimumPoweredTorque,
        .minimumDragTorque = minimumDragTorque,
        .minimumRecoverySlope = minimumRecoverySlope,
        .minimumRecoveryTime = minimumRecoveryTime,
        .minimumDriveTime = minimumDriveTime,
        .impulseDataArrayLength = impulseDataArrayLength,
        .driveHandleForcesMaxCapacity = driveHandleForcesMaxCapacity,
    };

    if (!eepromService.validateStrokePhaseDetectionSettings(newStrokeDetectionSettings))
    {
        return ResponseOpCodes::OperationFailed;
    }

    eepromService.setStrokePhaseDetectionSettings(newStrokeDetectionSettings);

    settingsBleService.broadcastStrokeDetectionSettings();

    return ResponseOpCodes::Successful;
}