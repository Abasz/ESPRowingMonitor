#pragma once

#include <array>
#include <vector>

#include "NimBLEDevice.h"

#include "../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../utils/enums.h"
#include "../../utils/ota-updater/ota-updater.service.interface.h"
#include "../sd-card/sd-card.service.interface.h"
#include "./ble-services/base-metrics.service.h"
#include "./ble-services/battery.service.h"
#include "./ble-services/extended-metrics.service.h"
#include "./ble-services/ota.service.h"
#include "./ble-services/settings.service.h"
#include "./bluetooth.controller.interface.h"
#include "./callbacks/server.callbacks.h"

using std::vector;

class BluetoothController final : public IBluetoothController
{
    friend class ChunkedNotifyMetricCallbacks;
    friend class ServerCallbacks;

    IEEPROMService &eepromService;
    ISdCardService &sdCardService;
    IOtaUploaderService &otaService;

    ServerCallbacks serverCallbacks;

    ExtendedMetricBleService extendedMetricsBleService;
    BaseMetricsBleService baseMetricsBleService;
    BatteryBleService batteryBleService;
    SettingsBleService settingsBleService;
    OtaBleService otaBleService;

    void setupBleDevice();
    void setupServices();
    void setupAdvertisement() const;

    std::array<unsigned char, SettingsBleService::settingsArrayLength> getSettings() const;

public:
    explicit BluetoothController(IEEPROMService &_eepromService, ISdCardService &_sdCardService, IOtaUploaderService &_otaService);

    void setup() override;
    void startBLEServer() override;
    void stopServer() override;

    void notifyBattery(unsigned char batteryLevel) const override;
    void notifyBaseMetrics(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) override;
    void notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor) override;
    void notifyHandleForces(const std::vector<float> &handleForces) override;
    void notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes) override;
    void notifySettings() const override;

    unsigned short getDeltaTimesMTU() const override;
    bool isDeltaTimesSubscribed() const override;
    bool isAnyDeviceConnected() override;
};