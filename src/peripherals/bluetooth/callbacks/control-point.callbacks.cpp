#include <array>
#include <limits>
#include <utility>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "globals.h"

#include "../../../utils/EEPROM/EEPROM.service.h"
#include "../bluetooth.controller.h"
#include "./control-point.callbacks.h"

using std::array;

ControlPointCallbacks::ControlPointCallbacks(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService) : settingsBleService(_settingsBleService), eepromService(_eepromService)
{
}

void ControlPointCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic, NimBLEConnInfo &connInfo)
{
    NimBLEAttValue message = pCharacteristic->getValue();

    Log.verboseln("Incoming connection");

    if (message.size() == 0)
    {
        Log.infoln("Invalid request, no Op Code");
        array<unsigned char, 3U> errorResponse = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(0),
            std::to_underlying(ResponseOpCodes::OperationFailed),
        };
        pCharacteristic->setValue(errorResponse);
        pCharacteristic->indicate();

        return;
    }

    Log.infoln("Op Code: %d; Length: %d", message[0], message.size());

    switch (message[0])
    {

    case std::to_underlying(SettingsOpCodes::SetLogLevel):
    {
        Log.infoln("Set LogLevel");

        const auto response = processLogLevel(message);

        array<unsigned char, 3U>
            temp = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                message[0],
                std::to_underlying(response),
            };

        pCharacteristic->setValue(temp);
    }
    break;

    case std::to_underlying(SettingsOpCodes::ChangeBleService):
    {
        Log.infoln("Change BLE Service");

        if (message.size() != 2 || !isInBounds(static_cast<unsigned int>(message[1]), 0U, 2U))
        {
            array<unsigned char, 3U> temp = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                message[0],
                std::to_underlying(ResponseOpCodes::InvalidParameter),
            };

            pCharacteristic->setValue(temp);

            break;
        }

        processBleServiceChange(message, pCharacteristic);

        return;
    }

    case std::to_underlying(SettingsOpCodes::SetSdCardLogging):
    {
        Log.infoln("Change Sd Card Logging");

        const auto response = processSdCardLogging(message);

        array<unsigned char, 3U> temp = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            message[0],
            std::to_underlying(response),
        };

        pCharacteristic->setValue(temp);
    }
    break;

    case std::to_underlying(SettingsOpCodes::SetDeltaTimeLogging):
    {
        Log.infoln("Change deltaTime logging");

        const auto response = processDeltaTimeLogging(message);

        array<unsigned char, 3U> temp = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            message[0],
            std::to_underlying(response),
        };

        pCharacteristic->setValue(temp);
    }
    break;

    case std::to_underlying(SettingsOpCodes::SetMachineSettings):
    {
        Log.infoln("Change Machine Settings");

        if constexpr (!Configurations::isRuntimeSettingsEnabled)
        {
            array<unsigned char, 3U> temp = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                message[0],
                std::to_underlying(ResponseOpCodes::UnsupportedOpCode),
            };

            pCharacteristic->setValue(temp);

            break;
        }

        const auto response = processMachineSettingsChange(message);

        array<unsigned char, 3U> temp = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            message[0],
            std::to_underlying(response),
        };

        pCharacteristic->setValue(temp);

        break;
    }

    case std::to_underlying(SettingsOpCodes::SetSensorSignalSettings):
    {
        Log.infoln("Change Sensor Signal Filter Settings");

        if constexpr (!Configurations::isRuntimeSettingsEnabled)
        {
            array<unsigned char, 3U> temp = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                message[0],
                std::to_underlying(ResponseOpCodes::UnsupportedOpCode),
            };

            pCharacteristic->setValue(temp);

            break;
        }

        const auto response = processSensorSignalSettingsChange(message);

        array<unsigned char, 3U> temp = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            message[0],
            std::to_underlying(response),
        };

        pCharacteristic->setValue(temp);

        break;
    }

    case std::to_underlying(SettingsOpCodes::SetDragFactorSettings):
    {
        Log.infoln("Change Drag Factor Settings");

        if constexpr (!Configurations::isRuntimeSettingsEnabled)
        {
            array<unsigned char, 3U> temp = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                message[0],
                std::to_underlying(ResponseOpCodes::UnsupportedOpCode),
            };

            pCharacteristic->setValue(temp);

            break;
        }

        const auto response = processDragFactorSettingsChange(message);

        array<unsigned char, 3U> temp = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            message[0],
            std::to_underlying(response),
        };

        pCharacteristic->setValue(temp);

        break;
    }

    case std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings):
    {
        Log.infoln("Change Stroke Detection Settings");

        if constexpr (!Configurations::isRuntimeSettingsEnabled)
        {
            array<unsigned char, 3U> temp = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                message[0],
                std::to_underlying(ResponseOpCodes::UnsupportedOpCode),
            };

            pCharacteristic->setValue(temp);

            break;
        }

        const auto response = processStrokeDetectionSettingsChange(message);

        array<unsigned char, 3U> temp = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            message[0],
            std::to_underlying(response),
        };

        pCharacteristic->setValue(temp);

        break;
    }

    case std::to_underlying(SettingsOpCodes::RestartDevice):
    {
        Log.verboseln("Restarting device...");

        array<unsigned char, 3U> temp = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            message[0],
            std::to_underlying(ResponseOpCodes::Successful),
        };

        pCharacteristic->setValue(temp);
        restartWithDelay(100);
    }
    break;

    default:
    {
        Log.infoln("Not Supported Op Code: %d", message[0]);
        array<unsigned char, 3U> response = {
            eepromService.getBleServiceFlag() == BleServiceFlag::FtmsService ? std::to_underlying(SettingsOpCodes::ResponseCodeFtms) : std::to_underlying(SettingsOpCodes::ResponseCode),
            message[0],
            std::to_underlying(eepromService.getBleServiceFlag() == BleServiceFlag::FtmsService ? ResponseOpCodes::ControlNotPermitted : ResponseOpCodes::UnsupportedOpCode),
        };
        pCharacteristic->setValue(response);
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

    if (!EEPROMService::validateMachineSettings(newMachineSettings))
    {
        return ResponseOpCodes::OperationFailed;
    }

    eepromService.setMachineSettings(newMachineSettings);

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processSensorSignalSettingsChange(const NimBLEAttValue &message)
{
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

    if (!EEPROMService::validateSensorSignalSettings(newSensorSignalSettings))
    {
        return ResponseOpCodes::OperationFailed;
    }

    eepromService.setSensorSignalSettings(newSensorSignalSettings);

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processDragFactorSettingsChange(const NimBLEAttValue &message)
{
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

    if (!EEPROMService::validateDragFactorSettings(newDragFactorSettings, eepromService.getSensorSignalSettings().rotationDebounceTimeMin))
    {
        return ResponseOpCodes::OperationFailed;
    }

    eepromService.setDragFactorSettings(newDragFactorSettings);

    settingsBleService.broadcastSettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processStrokeDetectionSettingsChange(const NimBLEAttValue &message)
{
    const auto opCodePayloadSize = 1U;
    if (message.size() != opCodePayloadSize + ISettingsBleService::strokeSettingsPayloadSize)
    {
        Log.infoln("Malformed OP command for stroke detection settings");

        return ResponseOpCodes::InvalidParameter;
    }
    auto bytePosition = opCodePayloadSize;

    const unsigned char impulseAndDetection = message[bytePosition];
    const auto strokeDetectionType = static_cast<StrokeDetectionType>(impulseAndDetection & 0x03);
    const unsigned char impulseDataArrayLength = impulseAndDetection >> 2U;
    bytePosition += ISettingsBleService::impulseAndDetectionTypePayloadSize;

    const float minimumPoweredTorque = static_cast<float>(static_cast<short>(message[bytePosition] | message[bytePosition + 1] << 8)) / ISettingsBleService::poweredTorqueScale;
    bytePosition += ISettingsBleService::poweredTorquePayloadSize;

    const float minimumDragTorque = static_cast<float>(static_cast<short>(message[bytePosition] | message[bytePosition + 1] << 8)) / ISettingsBleService::dragTorqueScale;
    bytePosition += ISettingsBleService::dragTorquePayloadSize;

    float minimumRecoverySlopeMargin = 0.0F;
    std::memcpy(&minimumRecoverySlopeMargin, std::next(message.data(), bytePosition), ISettingsBleService::recoverySlopeMarginPayloadSize);
    minimumRecoverySlopeMargin /= ISettingsBleService::recoverySlopeMarginPayloadScale;
    bytePosition += ISettingsBleService::recoverySlopeMarginPayloadSize;

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
        .minimumRecoverySlopeMargin = minimumRecoverySlopeMargin,
        .minimumRecoverySlope = minimumRecoverySlope,
        .minimumRecoveryTime = minimumRecoveryTime,
        .minimumDriveTime = minimumDriveTime,
        .impulseDataArrayLength = impulseDataArrayLength,
        .driveHandleForcesMaxCapacity = driveHandleForcesMaxCapacity,
    };

    if (!EEPROMService::validateStrokePhaseDetectionSettings(newStrokeDetectionSettings))
    {
        return ResponseOpCodes::OperationFailed;
    }

    eepromService.setStrokePhaseDetectionSettings(newStrokeDetectionSettings);

    settingsBleService.broadcastStrokeDetectionSettings();

    return ResponseOpCodes::Successful;
}