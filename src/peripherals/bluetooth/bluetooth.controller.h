#pragma once

#include <array>
#include <vector>

#include "NimBLEDevice.h"

#include "../../rower/stroke.model.h"
#include "../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../utils/enums.h"
#include "../../utils/ota-updater/ota-updater.service.interface.h"
#include "../sd-card/sd-card.service.interface.h"
#include "./ble-services/base-metrics.service.interface.h"
#include "./ble-services/battery.service.interface.h"
#include "./ble-services/device-info.service.interface.h"
#include "./ble-services/extended-metrics.service.interface.h"
#include "./ble-services/ota.service.interface.h"
#include "./ble-services/settings.service.interface.h"
#include "./bluetooth.controller.interface.h"
#include "./callbacks/server.callbacks.h"

using std::vector;

class BluetoothController final : public IBluetoothController
{
    IEEPROMService &eepromService;
    IOtaUpdaterService &otaService;

    ISettingsBleService &settingsBleService;
    IBatteryBleService &batteryBleService;
    IDeviceInfoBleService &deviceInfoBleService;
    IOtaBleService &otaBleService;
    IBaseMetricsBleService &baseMetricsBleService;
    IExtendedMetricBleService &extendedMetricsBleService;

    ServerCallbacks serverCallbacks;

    unsigned int lastMetricsBroadcastTime = 0UL;

    unsigned short bleRevTimeData = 0;
    unsigned int bleRevCountData = 0;
    unsigned short bleStrokeTimeData = 0;
    unsigned short bleStrokeCountData = 0;
    short bleAvgStrokePowerData = 0;

    void setupBleDevice();
    void setupServices();
    void setupAdvertisement() const;

public:
    explicit BluetoothController(IEEPROMService &_eepromService, IOtaUpdaterService &_otaService, ISettingsBleService &_settingsBleService, IBatteryBleService &_batteryBleService, IDeviceInfoBleService &_deviceInfoBleService, IOtaBleService &_otaBleService, IBaseMetricsBleService &_baseMetricsBleService, IExtendedMetricBleService &_extendedMetricsBleService);

    void update() override;

    void setup() override;
    void startBLEServer() override;
    void stopServer() override;

    void notifyBattery(unsigned char batteryLevel) const override;
    void notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes) override;
    void notifyNewMetrics(const RowingDataModels::RowingMetrics &data) override;

    unsigned short calculateDeltaTimesMtu() const override;
    bool isAnyDeviceConnected() override;
};