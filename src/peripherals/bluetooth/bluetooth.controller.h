#pragma once

#include <array>
#include <vector>

#include "NimBLEDevice.h"

#include "../../rower/stroke.model.h"
#include "../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../utils/enums.h"
#include "../../utils/ota-updater/ota-updater.service.interface.h"
#include "../sd-card/sd-card.service.interface.h"
#include "./ble-metrics.model.h"
#include "./ble-services/base-metrics.service.interface.h"
#include "./ble-services/battery.service.interface.h"
#include "./ble-services/device-info.service.interface.h"
#include "./ble-services/extended-metrics.service.interface.h"
#include "./ble-services/ota.service.interface.h"
#include "./ble-services/settings.service.interface.h"
#include "./bluetooth.controller.interface.h"
#include "./callbacks/connection-manager.callbacks.interface.h"

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

    IConnectionManagerCallbacks &connectionManagerCallbacks;

    unsigned int lastMetricsBroadcastTime = 0UL;
    unsigned int lastDeltaTimesBroadcastTime = 0UL;

    BleMetricsModel::BleMetricsData bleData = {};

    unsigned char minimumMtu = 100;

    vector<unsigned long> bleDeltaTimes;

    void setupBleDevice();
    void setupServices();
    void setupAdvertisement(const std::string &deviceName) const;
    void flushBleDeltaTimes(unsigned short mtu);

public:
    explicit BluetoothController(IEEPROMService &_eepromService, IOtaUpdaterService &_otaService, ISettingsBleService &_settingsBleService, IBatteryBleService &_batteryBleService, IDeviceInfoBleService &_deviceInfoBleService, IOtaBleService &_otaBleService, IBaseMetricsBleService &_baseMetricsBleService, IExtendedMetricBleService &_extendedMetricsBleService, IConnectionManagerCallbacks &_connectionManagerCallbacks);

    void update() override;

    void setup() override;
    void startBLEServer() override;
    void stopServer() override;

    void notifyBattery(unsigned char batteryLevel) const override;
    void notifyNewDeltaTime(unsigned long deltaTime) override;
    void notifyNewMetrics(const RowingDataModels::RowingMetrics &data) override;

    bool isAnyDeviceConnected() override;
};