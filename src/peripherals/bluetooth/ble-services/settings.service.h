#pragma once

#include "NimBLEDevice.h"

#include "../callbacks/control-point.callbacks.h"

class BluetoothController;

class SettingsBleService
{
public:
    static constexpr unsigned char settingsArrayLength = 1U;

    ControlPointCallbacks callbacks;
    NimBLECharacteristic *characteristic = nullptr;

    explicit SettingsBleService(BluetoothController &_bleController);

    NimBLEService *setup(NimBLEServer *server, std::array<unsigned char, settingsArrayLength> settings);
};
