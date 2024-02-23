#include "ArduinoLog.h"

#include "../utils/configuration.h"
#include "peripherals.controller.h"

PeripheralsController::PeripheralsController(BluetoothService &_bluetoothService, NetworkService &_networkService, SdCardService &_sdCardService, EEPROMService &_eepromService) : bluetoothService(_bluetoothService), networkService(_networkService), sdCardService(_sdCardService), eepromService(_eepromService)
{
    if constexpr (Configurations::enableWebSocketDeltaTimeLogging || (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC))
    {
        deltaTimes.reserve((Configurations::minimumRecoveryTime + Configurations::minimumDriveTime) / Configurations::rotationDebounceTimeMin);
    }
}

void PeripheralsController::update(const unsigned char batteryLevel)
{
    if constexpr (Configurations::isWebsocketEnabled)
    {
        networkService.update();
    }

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

    if constexpr (Configurations::isBleServiceEnabled)
    {
        const unsigned int bleUpdateInterval = 1'000;
        if (now - lastBroadcastTime > bleUpdateInterval)
        {
            notify();
            lastBroadcastTime = now;
        }
    }
}

void PeripheralsController::begin()
{
    Log.infoln("Setting up peripherals");

    if constexpr (Configurations::isWebsocketEnabled)
    {
        Log.infoln("Setting up Network service");
        networkService.setup();
    }

    if constexpr (Configurations::isBleServiceEnabled)
    {
        Log.infoln("Setting up BLE service");
        bluetoothService.setup();
    }

    if constexpr (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC)
    {
        Log.infoln("Setting up SDCard service");
        sdCardService.setup();
    }

    if constexpr (Configurations::ledPin != GPIO_NUM_NC)
    {
        setupConnectionIndicatorLed();
    }
}

bool PeripheralsController::isAnyDeviceConnected()
{
    return (Configurations::isBleServiceEnabled && BluetoothService::isAnyDeviceConnected()) ||
           (Configurations::isWebsocketEnabled && networkService.isAnyDeviceConnected());
}

void PeripheralsController::updateLed(const CRGB::HTMLColorCode newLedColor)
{
    if constexpr (Configurations::isRgb)
    {
        ledColor = isAnyDeviceConnected() ? newLedColor : ledColor == CRGB::Black ? newLedColor
                                                                                  : CRGB::Black;
        leds[0] = ledColor;
        FastLED.show();
    }
    else
    {
        ledState = isAnyDeviceConnected() ? HIGH : ledState == HIGH ? LOW
                                                                    : HIGH;

        digitalWrite(Configurations::ledPin, ledState);
    }
}

void PeripheralsController::notifyBattery(const unsigned char batteryLevel)
{
    if constexpr (Configurations::isBleServiceEnabled)
    {
        bluetoothService.notifyBattery(batteryLevel);
    }

    if constexpr (Configurations::isWebsocketEnabled)
    {
        networkService.notifyBatteryLevel(batteryLevel);
    }
}

void PeripheralsController::updateDeltaTime(const unsigned long deltaTime)
{
    if constexpr (Configurations::enableWebSocketDeltaTimeLogging || (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC))
    {
        if (eepromService.getLogToSdCard() || (eepromService.getLogToWebsocket() && networkService.isAnyDeviceConnected()))
        {
            deltaTimes.push_back(deltaTime);
        }
    }
}

void PeripheralsController::updateData(const RowingDataModels::RowingMetrics &data)
{
    const auto secInMicroSec = 1e6L;
    bleRevTimeData = lroundl((data.lastRevTime / secInMicroSec) * (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService ? 2'048 : 1'024)) % USHRT_MAX;
    bleRevCountData = lround(data.distance);
    bleStrokeTimeData = lroundl((data.lastStrokeTime / secInMicroSec) * 1'024) % USHRT_MAX;
    bleStrokeCountData = data.strokeCount;
    bleAvgStrokePowerData = static_cast<short>(lround(data.avgStrokePower));

    if constexpr (Configurations::isWebsocketEnabled)
    {
        networkService.notifyClients(data, Configurations::enableWebSocketDeltaTimeLogging ? deltaTimes : vector<unsigned long>{});
    }

    if constexpr (Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC)
    {
        sdCardService.saveDeltaTime(deltaTimes);
    }

    if constexpr ((Configurations::supportSdCardLogging && Configurations::sdCardChipSelectPin != GPIO_NUM_NC) || Configurations::isWebsocketEnabled)
    {
        if (deltaTimes.size() > 0)
        {
            vector<unsigned long> clear;
            clear.reserve((Configurations::minimumRecoveryTime + Configurations::minimumDriveTime) / Configurations::rotationDebounceTimeMin);
            deltaTimes.swap(clear);
        }
    }
}

void PeripheralsController::notify() const
{
    if constexpr (Configurations::isBleServiceEnabled)
    {
        if (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService)
        {
            bluetoothService.notifyPsc(bleRevTimeData, bleRevCountData, bleStrokeTimeData, bleStrokeCountData, bleAvgStrokePowerData);
        }
        if (eepromService.getBleServiceFlag() == BleServiceFlag::CscService)
        {
            bluetoothService.notifyCsc(bleRevTimeData, bleRevCountData, bleStrokeTimeData, bleStrokeCountData);
        }
    }
}

void PeripheralsController::notifyDragFactor(const unsigned char dragFactor) const
{
    if constexpr (Configurations::isBleServiceEnabled)
    {
        const auto distance = pow(dragFactor / Configurations::concept2MagicNumber, 1.0 / 3.0) * (2.0 * PI) * 10;
        bluetoothService.notifyDragFactor(static_cast<unsigned short>(distance), dragFactor);
    }
}

void PeripheralsController::setupConnectionIndicatorLed()
{
    if constexpr (Configurations::isRgb)
    {
        FastLED.addLeds<WS2812, static_cast<unsigned char>(Configurations::ledPin), GRB>(leds.data(), 1);
    }
    else
    {
        pinMode(Configurations::ledPin, OUTPUT);
    }
}