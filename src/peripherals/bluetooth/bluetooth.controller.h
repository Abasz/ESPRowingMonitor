#pragma once

#include <array>
#include <vector>

#include "NimBLEDevice.h"

#include "../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../utils/enums.h"
#include "../../utils/ota-updater/ota-updater.service.interface.h"
#include "../sd-card/sd-card.service.interface.h"
#include "./ble-services/base-metrics.service.h"
#include "./ble-services/battery.service.interface.h"
#include "./ble-services/extended-metrics.service.h"
#include "./ble-services/ota.service.h"
#include "./ble-services/settings.service.interface.h"
#include "./ble-services/device-info.service.interface.h"
#include "./bluetooth.controller.interface.h"
#include "./callbacks/server.callbacks.h"

using std::vector;

class BluetoothController final : public IBluetoothController
{
    IEEPROMService &eepromService;
    IOtaUploaderService &otaService;

    ISettingsBleService &settingsBleService;
    IBatteryBleService &batteryBleService;
    IDeviceInfoBleService &deviceInfoBleService;

    ExtendedMetricBleService extendedMetricsBleService;
    BaseMetricsBleService baseMetricsBleService;
    OtaBleService otaBleService;

    ServerCallbacks serverCallbacks;

    void setupBleDevice();
    void setupServices();
    void setupAdvertisement() const;

public:
    explicit BluetoothController(IEEPROMService &_eepromService, IOtaUploaderService &_otaService, ISettingsBleService &_settingsBleService, IBatteryBleService &_batteryBleService,IDeviceInfoBleService &_deviceInfoBleService);

    void setup() override;
    void startBLEServer() override;
    void stopServer() override;

    void notifyBattery(unsigned char batteryLevel) const override;
    void notifyBaseMetrics(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) override;
    void notifyExtendedMetrics(short avgStrokePower, unsigned int recoveryDuration, unsigned int driveDuration, unsigned char dragFactor) override;
    void notifyHandleForces(const std::vector<float> &handleForces) override;
    void notifyDeltaTimes(const std::vector<unsigned long> &deltaTimes) override;

    unsigned short getDeltaTimesMTU() const override;
    bool isDeltaTimesSubscribed() const override;
    bool isAnyDeviceConnected() override;
};