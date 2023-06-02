#include "ArduinoLog.h"

#include "../utils/configuration.h"
#include "peripherals.controller.h"

PeripheralsController::PeripheralsController(BluetoothService &_bluetoothService, NetworkService &_networkService, EEPROMService &_eepromService) : bluetoothService(_bluetoothService), networkService(_networkService), eepromService(_eepromService)
{
}

void PeripheralsController::update()
{
    if constexpr (Configurations::isWebsocketEnabled)
    {
        networkService.update();
    }

    auto const now = millis();
    if (now - lastConnectedDeviceCheckTime > Configurations::ledBlinkFrequency)
    {
        updateLed();
        lastConnectedDeviceCheckTime = now;
    }

    if constexpr (Configurations::isBleSErviceEnabled)
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

    if constexpr (Configurations::isBleSErviceEnabled)
    {
        bluetoothService.setup();
    }

    setupConnectionIndicatorLed();
}

bool PeripheralsController::isAnyDeviceConnected()
{
    return BluetoothService::isAnyDeviceConnected() || networkService.isAnyDeviceConnected();
}

void PeripheralsController::updateLed()
{
    ledState = isAnyDeviceConnected() ? HIGH : ledState == HIGH ? LOW
                                                                : HIGH;

    digitalWrite(GPIO_NUM_2, ledState);
}

void PeripheralsController::notifyBattery(const unsigned char batteryLevel)
{
    batteryLevelData = batteryLevel;
    if constexpr (Configurations::isBleSErviceEnabled)
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
    if constexpr (Configurations::isBleSErviceEnabled)
    {
        auto const distance = pow(dragFactor / Configurations::concept2MagicNumber, 1.0 / 3.0) * (2.0 * PI) * 10;
        bluetoothService.notifyDragFactor(static_cast<unsigned short>(distance), dragFactor);
    }
}

void PeripheralsController::setupConnectionIndicatorLed() const
{
    pinMode(GPIO_NUM_2, OUTPUT);
}