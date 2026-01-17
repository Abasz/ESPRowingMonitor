#include <utility>

#include "ArduinoLog.h"

#include "../utils/configuration.h"
#include "./peripherals.controller.h"

PeripheralsController::PeripheralsController(IBluetoothController &_bluetoothController, ISdCardService &_sdCardService, IEEPROMService &_eepromService, ILedService &_ledService)
    : bluetoothController(_bluetoothController),
      sdCardService(_sdCardService),
      eepromService(_eepromService),
      ledService(_ledService)
{
    if constexpr ((Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC))
    {
        sdDeltaTimes.reserve((RowerProfile::Defaults::minimumRecoveryTime + RowerProfile::Defaults::minimumDriveTime) / RowerProfile::Defaults::rotationDebounceTimeMin);
    }
}

void PeripheralsController::update(const unsigned char batteryLevel)
{
    bluetoothController.update();

    const auto now = millis();
    if constexpr (Configurations::ledPin != GPIO_NUM_NC)
    {
        if (now - lastConnectedDeviceCheckTime > Configurations::ledBlinkFrequency)
        {
            auto ledColor = LedColor::Blue;
            const auto minBattLevel = 30;
            if (batteryLevel < minBattLevel)
            {
                ledColor = LedColor::Red;
            }
            const auto maxBattLevel = 80;
            if (batteryLevel > maxBattLevel)
            {
                ledColor = LedColor::Green;
            }
            updateLed(ledColor);
            lastConnectedDeviceCheckTime = now;
        }
    }
}

void PeripheralsController::begin()
{
    if constexpr (Configurations::isRuntimeSettingsEnabled)
    {
        rotationDebounceTimeMin = eepromService.getSensorSignalSettings().rotationDebounceTimeMin;

        const auto strokePhaseDetectionSettings = eepromService.getStrokePhaseDetectionSettings();
        minimumRecoveryTime = strokePhaseDetectionSettings.minimumRecoveryTime;
        minimumDriveTime = strokePhaseDetectionSettings.minimumDriveTime;

        sdDeltaTimes.clear();
        sdDeltaTimes.shrink_to_fit();
        sdDeltaTimes.reserve((minimumRecoveryTime + minimumDriveTime) / rotationDebounceTimeMin);
    }

    Log.infoln("Setting up peripherals");

    if constexpr (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC)
    {
        Log.infoln("Setting up SDCard service");
        sdCardService.setup();
    }

    Log.infoln("Setting up BLE service");
    bluetoothController.setup();
}

bool PeripheralsController::isAnyDeviceConnected()
{
    return bluetoothController.isAnyDeviceConnected();
}

void PeripheralsController::updateLed(const LedColor newLedColor)
{
    const auto previousLedState = ledService.getColor();
    const auto isConnected = isAnyDeviceConnected();

    if (isConnected && newLedColor == previousLedState)
    {
        return;
    }

    ledService.setColor(isConnected || previousLedState == LedColor::Black ? newLedColor : LedColor::Black);

    ledService.refresh();
}

void PeripheralsController::notifyBattery(const unsigned char batteryLevel)
{
    bluetoothController.notifyBattery(batteryLevel);
}

void PeripheralsController::updateDeltaTime(const unsigned long deltaTime)
{
    if constexpr (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC)
    {
        if ((eepromService.getLogToSdCard() && sdCardService.isLogFileOpen()))
        {
            sdDeltaTimes.push_back(deltaTime);
        }
    }

    if constexpr (Configurations::enableBluetoothDeltaTimeLogging)
    {
        if (eepromService.getLogToBluetooth())
        {
            bluetoothController.notifyNewDeltaTime(deltaTime);
        }
    }
}

void PeripheralsController::updateData(const RowingDataModels::RowingMetrics &data)
{
    bluetoothController.notifyNewMetrics(data);

    if constexpr (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC)
    {
        if (eepromService.getLogToSdCard())
        {
            sdCardService.saveDeltaTime(sdDeltaTimes);
        }

        if (!sdDeltaTimes.empty())
        {
            vector<unsigned long> clear;
            clear.reserve((minimumRecoveryTime + minimumDriveTime) / rotationDebounceTimeMin);
            sdDeltaTimes.swap(clear);
        }
    }
}
