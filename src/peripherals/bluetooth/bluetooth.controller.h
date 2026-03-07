#pragma once

#include <string>
#include <vector>

#include "./ble-metrics.model.h"
#include "./bluetooth.controller.interface.h"

class IBaseMetricsBleService;
class IBatteryBleService;
class IConnectionManagerCallbacks;
class IDeviceInfoBleService;
class IEEPROMService;
class IExtendedMetricBleService;
class IOtaBleService;
class IOtaUpdaterService;
class ISettingsBleService;

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