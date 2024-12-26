#include "ArduinoLog.h"

#include "../utils/configuration.h"
#include "./peripherals.controller.h"

PeripheralsController::PeripheralsController(IBluetoothController &_bluetoothController, ISdCardService &_sdCardService, IEEPROMService &_eepromService) : bluetoothController(_bluetoothController), sdCardService(_sdCardService), eepromService(_eepromService)
{
    if constexpr ((Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC))
    {
        sdDeltaTimes.reserve((Configurations::minimumRecoveryTime + Configurations::minimumDriveTime) / Configurations::rotationDebounceTimeMin);
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
            auto ledColor = CRGB::Blue;
            const auto minBattLevel = 30;
            if (batteryLevel < minBattLevel)
            {
                ledColor = CRGB::Red;
            }
            const auto maxBattLevel = 80;
            if (batteryLevel > maxBattLevel)
            {
                ledColor = CRGB::Green;
            }
            updateLed(ledColor);
            lastConnectedDeviceCheckTime = now;
        }
    }
}

void PeripheralsController::begin()
{
    Log.infoln("Setting up peripherals");

    if constexpr (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC)
    {
        Log.infoln("Setting up SDCard service");
        sdCardService.setup();
    }

    Log.infoln("Setting up BLE service");
    bluetoothController.setup();

    if constexpr (Configurations::ledPin != GPIO_NUM_NC)
    {
        setupConnectionIndicatorLed();
    }
}

bool PeripheralsController::isAnyDeviceConnected()
{
    return bluetoothController.isAnyDeviceConnected();
}

void PeripheralsController::updateLed(const CRGB::HTMLColorCode newLedColor)
{
    if constexpr (Configurations::isRgb)
    {
        const auto previousLedState = static_cast<CRGB>(leds[0]);
        const auto isConnected = isAnyDeviceConnected();

        if (isConnected && newLedColor == previousLedState)
        {
            return;
        }

        leds[0] = isConnected || previousLedState == CRGB::Black ? newLedColor : CRGB::Black;

        FastLED.show();

        return;
    }

    ledState = isAnyDeviceConnected() ? HIGH : (ledState ^ HIGH);

    digitalWrite(Configurations::ledPin, ledState);
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
    }

    if constexpr (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC)
    {
        if (!sdDeltaTimes.empty())
        {
            vector<unsigned long> clear;
            clear.reserve((Configurations::minimumRecoveryTime + Configurations::minimumDriveTime) / Configurations::rotationDebounceTimeMin);
            sdDeltaTimes.swap(clear);
        }
    }
}

void PeripheralsController::setupConnectionIndicatorLed()
{
    if constexpr (Configurations::isRgb)
    {
        FastLED.addLeds<WS2812, static_cast<unsigned char>(Configurations::ledPin), Configurations::ledColorChannelOrder>(leds.data(), 1);
    }
    else
    {
        pinMode(Configurations::ledPin, OUTPUT);
    }
}