#pragma once
#include <vector>

#include "../rower/stroke.model.h"
#include "../utils/EEPROM/EEPROM.service.interface.h"
#include "./bluetooth/bluetooth.controller.interface.h"
#include "./led/led.service.interface.h"
#include "./peripherals.controller.interface.h"
#include "./sd-card/sd-card.service.interface.h"

using std::vector;

class PeripheralsController final : public IPeripheralsController
{
    IBluetoothController &bluetoothController;
    ISdCardService &sdCardService;
    IEEPROMService &eepromService;
    ILedService &ledService;

    unsigned short rotationDebounceTimeMin = RowerProfile::Defaults::rotationDebounceTimeMin;
    unsigned int minimumRecoveryTime = RowerProfile::Defaults::minimumRecoveryTime;
    unsigned int minimumDriveTime = RowerProfile::Defaults::minimumDriveTime;

    unsigned int lastConnectedDeviceCheckTime = 0;

    vector<unsigned long> sdDeltaTimes;

    void updateLed(LedColor newLedColor);

public:
    PeripheralsController(IBluetoothController &_bluetoothController, ISdCardService &sdCardService, IEEPROMService &_eepromService, ILedService &_ledService);

    void begin() override;
    void update(unsigned char batteryLevel) override;
    void notifyBattery(unsigned char batteryLevel) override;
    void updateData(const RowingDataModels::RowingMetrics &data) override;
    void updateDeltaTime(unsigned long deltaTime) override;
    bool isAnyDeviceConnected() override;
};
