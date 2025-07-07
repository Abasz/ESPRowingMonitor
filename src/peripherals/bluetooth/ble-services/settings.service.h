#pragma once

#include <array>

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../sd-card/sd-card.service.interface.h"
#include "../bluetooth.controller.interface.h"
#include "../callbacks/control-point.callbacks.h"
#include "./settings.service.interface.h"

class SettingsBleService final : public ISettingsBleService
{
    ISdCardService &sdCardService;
    IEEPROMService &eepromService;

    ControlPointCallbacks callbacks;
    NimBLECharacteristic *settingsCharacteristic = nullptr;
    NimBLECharacteristic *strokeSettingsCharacteristic = nullptr;

    std::array<unsigned char, ISettingsBleService::settingsPayloadSize> getSettings() const;
    std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize> getStrokeDetectionSettings() const;

public:
    explicit SettingsBleService(ISdCardService &_sdCardService, IEEPROMService &_eepromService);

    NimBLEService *setup(NimBLEServer *server) override;
    void broadcastSettings() const override;
    void broadcastStrokeDetectionSettings() const override;
};
