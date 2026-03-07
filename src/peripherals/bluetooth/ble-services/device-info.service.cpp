#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "globals.h"

#include "./device-info.service.h"

#include "../../../utils/configuration.h"
#include "../ble.enums.h"

NimBLEService *DeviceInfoBleService::setup(NimBLEServer *const server) const
{
    Log.traceln("Setting up Device Info Service");
    auto *const deviceInfoService = server->createService(CommonBleFlags::deviceInfoSvcUuid);

    deviceInfoService
        ->createCharacteristic(CommonBleFlags::manufacturerNameSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::deviceName);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::modelNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::modelNumber);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::serialNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::serialNumber.empty() ? generateSerial() : Configurations::serialNumber);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::firmwareNumberSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::firmwareVersion);
    deviceInfoService
        ->createCharacteristic(CommonBleFlags::hardwareRevisionSvcUuid, NIMBLE_PROPERTY::READ)
        ->setValue(Configurations::hardwareRevision);

    return deviceInfoService;
}