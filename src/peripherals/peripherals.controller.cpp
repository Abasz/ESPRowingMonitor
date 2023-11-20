#include "ArduinoLog.h"

#include "../utils/configuration.h"
#include "peripherals.controller.h"

PeripheralsController::PeripheralsController(BluetoothService &_bluetoothService, NetworkService &_networkService, EEPROMService &_eepromService) : bluetoothService(_bluetoothService), networkService(_networkService), eepromService(_eepromService)
{
}

void PeripheralsController::update(unsigned char batteryLevel)
{
    if constexpr (Configurations::isWebsocketEnabled)
    {
        networkService.update();
    }

    auto const now = millis();
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
        if (now - lastBroadcastTime > updateInterval)
        {
            notify();
            lastBroadcastTime = now;
        }
    }
}

void PeripheralsController::begin()
{
    Log.infoln("Setting up BLE Controller");

    if constexpr (Configurations::isWebsocketEnabled)
    {
        networkService.setup();
    }

    if constexpr (Configurations::isBleServiceEnabled)
    {
        bluetoothService.setup();
    }

    if constexpr (Configurations::ledPin != GPIO_NUM_NC)
    {
        setupConnectionIndicatorLed();
    }
}

bool PeripheralsController::isAnyDeviceConnected()
{
    return BluetoothService::isAnyDeviceConnected() || networkService.isAnyDeviceConnected();
}

void PeripheralsController::updateLed(CRGB::HTMLColorCode newLedColor)
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
    batteryLevelData = batteryLevel;
    if constexpr (Configurations::isBleServiceEnabled)
    {
        bluetoothService.notifyBattery(batteryLevel);
    }
}

void PeripheralsController::updateData(const RowingDataModels::RowingMetrics data)
{
    auto const secInMicroSec = 1e6L;
    bleRevTimeData = lroundl((data.lastRevTime / secInMicroSec) * (eepromService.getBleServiceFlag() == BleServiceFlag::CpsService ? 2048 : 1024)) % USHRT_MAX;
    bleRevCountData = lround(data.distance);
    bleStrokeTimeData = lroundl((data.lastStrokeTime / secInMicroSec) * 1024) % USHRT_MAX;
    bleStrokeCountData = data.strokeCount;
    bleAvgStrokePowerData = static_cast<short>(lround(data.avgStrokePower));

    if constexpr (Configurations::isWebsocketEnabled)
    {
        networkService.notifyClients(data, batteryLevelData, eepromService.getBleServiceFlag(), eepromService.getLogLevel());
    }
}

void PeripheralsController::notify() const
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

void PeripheralsController::notifyDragFactor(const unsigned char dragFactor) const
{
    if constexpr (Configurations::isBleServiceEnabled)
    {
        auto const distance = pow(dragFactor / Configurations::concept2MagicNumber, 1.0 / 3.0) * (2.0 * PI) * 10;
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