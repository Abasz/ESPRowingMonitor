#pragma once

#include <array>

#include "../callbacks/control-point.callbacks.h"
#include "./settings.service.interface.h"

class IEEPROMService;
class ISdCardService;
class NimBLECharacteristic;
class NimBLEServer;
class NimBLEService;

class SettingsBleService final : public ISettingsBleService
{
    ISdCardService &sdCardService;
    IEEPROMService &eepromService;

    ControlPointCallbacks callbacks;
    NimBLECharacteristic *settingsCharacteristic = nullptr;
    NimBLECharacteristic *strokeSettingsCharacteristic = nullptr;

    [[nodiscard]] std::array<unsigned char, ISettingsBleService::settingsPayloadSize> getSettings() const;
    [[nodiscard]] std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize> getStrokeDetectionSettings() const;

public:
    explicit SettingsBleService(ISdCardService &_sdCardService, IEEPROMService &_eepromService);

    NimBLEService *setup(NimBLEServer *server) override;
    void broadcastSettings() const override;
    void broadcastStrokeDetectionSettings() const override;
};
