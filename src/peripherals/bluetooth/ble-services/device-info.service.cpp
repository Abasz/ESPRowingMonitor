#include <array>

#include "ArduinoLog.h"

#include "../../../utils/configuration.h"
#include "../../../utils/enums.h"
#include "./device-info.service.h"

NimBLEService *DeviceInfoBleService::setup(NimBLEServer *const server)
{
    Log.traceln("Setting up Device Info Service");
    auto *deviceInfoService = server->createService(CommonBleFlags::deviceInfoSvcUuid);

    deviceInfoService
        ->createCharacteristic(CommonBleFlags::manufacturerNameSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::deviceName);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::modelNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::modelNumber);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::serialNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::serialNumber);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::firmwareNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::firmwareVersion);

    return deviceInfoService;
}