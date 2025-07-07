#include <array>
#include <bit>
#include <utility>

#include "ArduinoLog.h"

#include "../../../utils/configuration.h"
#include "../../../utils/enums.h"
#include "../ble-metrics.model.h"
#include "./settings.service.h"

SettingsBleService::SettingsBleService(ISdCardService &_sdCardService, IEEPROMService &_eepromService) : sdCardService(_sdCardService), eepromService(_eepromService), callbacks(*this, _eepromService)
{
}

NimBLEService *SettingsBleService::setup(NimBLEServer *const server)
{
    Log.traceln("Setting up Settings Service");
    auto *settingsService = server->createService(CommonBleFlags::settingsServiceUuid);
    settingsCharacteristic = settingsService->createCharacteristic(CommonBleFlags::settingsUuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    strokeSettingsCharacteristic = settingsService->createCharacteristic(CommonBleFlags::strokeDetectionSettingsUuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);

    settingsCharacteristic->setValue(getSettings());
    strokeSettingsCharacteristic->setValue(getStrokeDetectionSettings());

    settingsService->createCharacteristic(CommonBleFlags::settingsControlPointUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE)->setCallbacks(&callbacks);

    return settingsService;
}

void SettingsBleService::broadcastSettings() const
{
    ASSERT_SETUP_CALLED(settingsCharacteristic);

    settingsCharacteristic->setValue(getSettings());
    settingsCharacteristic->notify();
}

void SettingsBleService::broadcastStrokeDetectionSettings() const
{
    ASSERT_SETUP_CALLED(strokeSettingsCharacteristic);

    strokeSettingsCharacteristic->setValue(getStrokeDetectionSettings());
    strokeSettingsCharacteristic->notify();
}

std::array<unsigned char, ISettingsBleService::settingsPayloadSize> SettingsBleService::getSettings() const
{
    const unsigned char baseSettings =
        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(eepromService.getLogToBluetooth()) + 1 : 0) << 0U) |
        ((Configurations::supportSdCardLogging && sdCardService.isLogFileOpen() ? static_cast<unsigned char>(eepromService.getLogToSdCard()) + 1 : 0) << 2U) |
        (std::to_underlying(eepromService.getLogLevel()) << 4U) |
        (static_cast<unsigned char>(Configurations::isRuntimeSettingsEnabled) << 7U);

    const auto machineSettings = eepromService.getMachineSettings();

    const auto flywheelInertia = std::bit_cast<unsigned int>(machineSettings.flywheelInertia);
    const auto mToCm = 100.F;
    const auto sprocketRadius = static_cast<unsigned short>(roundf(machineSettings.sprocketRadius * ISettingsBleService::sprocketRadiusScale * mToCm));

    const auto sensorSignalSettings = eepromService.getSensorSignalSettings();

    const auto dragFactorSettings = eepromService.getDragFactorSettings();

    const auto dragFactorLowerThreshold = static_cast<unsigned short>(roundf(dragFactorSettings.lowerDragFactorThreshold * ISettingsBleService::dragFactorThresholdScale));
    const auto dragFactorUpperThreshold = static_cast<unsigned short>(roundf(dragFactorSettings.upperDragFactorThreshold * ISettingsBleService::dragFactorThresholdScale));

    std::array<unsigned char, ISettingsBleService::settingsPayloadSize>
        temp = {
            baseSettings,
            static_cast<unsigned char>(flywheelInertia),
            static_cast<unsigned char>(flywheelInertia >> 8),
            static_cast<unsigned char>(flywheelInertia >> 16),
            static_cast<unsigned char>(flywheelInertia >> 24),
            static_cast<unsigned char>(roundf(machineSettings.concept2MagicNumber * ISettingsBleService::magicNumberScale)),
            machineSettings.impulsesPerRevolution,
            static_cast<unsigned char>(sprocketRadius),
            static_cast<unsigned char>(sprocketRadius >> 8),
            static_cast<unsigned char>(sensorSignalSettings.rotationDebounceTimeMin / ISettingsBleService::debounceTimeScale),
            static_cast<unsigned char>(sensorSignalSettings.rowingStoppedThresholdPeriod / ISettingsBleService::rowingStoppedThresholdScale),
            static_cast<unsigned char>(roundf(dragFactorSettings.goodnessOfFitThreshold * ISettingsBleService::goodnessOfFitThresholdScale)),
            static_cast<unsigned char>(dragFactorSettings.maxDragFactorRecoveryPeriod / ISettingsBleService::dragFactorRecoveryPeriodScale),
            static_cast<unsigned char>(dragFactorLowerThreshold),
            static_cast<unsigned char>(dragFactorLowerThreshold >> 8),
            static_cast<unsigned char>(dragFactorUpperThreshold),
            static_cast<unsigned char>(dragFactorUpperThreshold >> 8),
            dragFactorSettings.dragCoefficientsArrayLength,
        };

    return temp;
}

std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize> SettingsBleService::getStrokeDetectionSettings() const
{
    const auto strokeDetectionSettings = eepromService.getStrokePhaseDetectionSettings();

    const unsigned char impulseAndDetection = (std::to_underlying(strokeDetectionSettings.strokeDetectionType) & 0x03) | (strokeDetectionSettings.impulseDataArrayLength << 2U);
    const auto poweredTorque = static_cast<short>(roundf(strokeDetectionSettings.minimumPoweredTorque * ISettingsBleService::poweredTorqueScale));
    const auto dragTorque = static_cast<short>(roundf(strokeDetectionSettings.minimumDragTorque * ISettingsBleService::dragTorqueScale));
    const auto recoverySlopeMargin = std::bit_cast<unsigned int>(strokeDetectionSettings.minimumRecoverySlopeMargin * ISettingsBleService::recoverySlopeMarginPayloadScale);
    const auto recoverySlope = static_cast<short>(roundf(strokeDetectionSettings.minimumRecoverySlope * ISettingsBleService::recoverySlopeScale));
    const auto strokeTimes = (strokeDetectionSettings.minimumRecoveryTime / ISettingsBleService::minimumStrokeTimesScale) | ((strokeDetectionSettings.minimumDriveTime / ISettingsBleService::minimumStrokeTimesScale) << 12);

    std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize>
        temp = {
            impulseAndDetection,
            static_cast<unsigned char>(poweredTorque),
            static_cast<unsigned char>(poweredTorque >> 8),
            static_cast<unsigned char>(dragTorque),
            static_cast<unsigned char>(dragTorque >> 8),
            static_cast<unsigned char>(recoverySlopeMargin),
            static_cast<unsigned char>(recoverySlopeMargin >> 8),
            static_cast<unsigned char>(recoverySlopeMargin >> 16),
            static_cast<unsigned char>(recoverySlopeMargin >> 24),
            static_cast<unsigned char>(recoverySlope),
            static_cast<unsigned char>(recoverySlope >> 8),
            static_cast<unsigned char>(strokeTimes),
            static_cast<unsigned char>(strokeTimes >> 8),
            static_cast<unsigned char>(strokeTimes >> 16),
            strokeDetectionSettings.driveHandleForcesMaxCapacity,
        };

    return temp;
}