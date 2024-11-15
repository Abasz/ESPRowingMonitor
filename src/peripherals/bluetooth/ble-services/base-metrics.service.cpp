#include <array>

#include "ArduinoLog.h"

#include "./base-metrics.service.h"
#include "./settings.service.interface.h"

using std::array;

BaseMetricsBleService::BaseMetricsBleService(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService) : callbacks(_settingsBleService, _eepromService)
{
}

NimBLEService *BaseMetricsBleService::setup(NimBLEServer *server, const BleServiceFlag bleServiceFlag)
{
    return bleServiceFlag == BleServiceFlag::CscService ? setupCscServices(server) : setupPscServices(server);
}

void BaseMetricsBleService::cscTask(void *parameters)
{
    {
        const auto *const params = static_cast<const BaseMetricsBleService::BaseMetricsParams *>(parameters);
        const auto length = 11U;
        array<unsigned char, length> temp = {
            CSCSensorBleFlags::cscMeasurementFeaturesFlag,

            static_cast<unsigned char>(params->revCount),
            static_cast<unsigned char>(params->revCount >> 8),
            static_cast<unsigned char>(params->revCount >> 16),
            static_cast<unsigned char>(params->revCount >> 24),

            static_cast<unsigned char>(params->revTime),
            static_cast<unsigned char>(params->revTime >> 8),

            static_cast<unsigned char>(params->strokeCount),
            static_cast<unsigned char>(params->strokeCount >> 8),
            static_cast<unsigned char>(params->strokeTime),
            static_cast<unsigned char>(params->strokeTime >> 8)};
        params->characteristic->setValue(temp);
        params->characteristic->notify();
    }
    vTaskDelete(NULL);
}

void BaseMetricsBleService::pscTask(void *parameters)
{
    {
        const auto *const params = static_cast<const BaseMetricsBleService::BaseMetricsParams *>(parameters);

        const auto length = 14U;
        array<unsigned char, length> temp = {
            static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag),
            static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag >> 8),

            static_cast<unsigned char>(params->avgStrokePower),
            static_cast<unsigned char>(params->avgStrokePower >> 8),

            static_cast<unsigned char>(params->revCount),
            static_cast<unsigned char>(params->revCount >> 8),
            static_cast<unsigned char>(params->revCount >> 16),
            static_cast<unsigned char>(params->revCount >> 24),
            static_cast<unsigned char>(params->revTime),
            static_cast<unsigned char>(params->revTime >> 8),

            static_cast<unsigned char>(params->strokeCount),
            static_cast<unsigned char>(params->strokeCount >> 8),
            static_cast<unsigned char>(params->strokeTime),
            static_cast<unsigned char>(params->strokeTime >> 8),
        };

        params->characteristic->setValue(temp);
        params->characteristic->notify();
    }
    vTaskDelete(NULL);
}

NimBLEService *BaseMetricsBleService::setupCscServices(NimBLEServer *const server)
{
    Log.infoln("Setting up Cycling Speed and Cadence Profile");

    auto *cscService = server->createService(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid);
    parameters.characteristic = cscService->createCharacteristic(CSCSensorBleFlags::cscMeasurementUuid, NIMBLE_PROPERTY::NOTIFY);

    cscService
        ->createCharacteristic(CSCSensorBleFlags::cscFeatureUuid, NIMBLE_PROPERTY::READ)
        ->setValue(CSCSensorBleFlags::cscFeaturesFlag);

    cscService
        ->createCharacteristic(CommonBleFlags::sensorLocationUuid, NIMBLE_PROPERTY::READ)
        ->setValue(CommonBleFlags::sensorLocationFlag);

    cscService->createCharacteristic(CSCSensorBleFlags::cscControlPointUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE)->setCallbacks(&callbacks);

    return cscService;
}

NimBLEService *BaseMetricsBleService::setupPscServices(NimBLEServer *const server)
{
    Log.infoln("Setting up Cycling Power Profile");
    auto *pscService = server->createService(PSCSensorBleFlags::cyclingPowerSvcUuid);
    parameters.characteristic = pscService->createCharacteristic(PSCSensorBleFlags::pscMeasurementUuid, NIMBLE_PROPERTY::NOTIFY);

    pscService
        ->createCharacteristic(PSCSensorBleFlags::pscFeatureUuid, NIMBLE_PROPERTY::READ)
        ->setValue(PSCSensorBleFlags::pscFeaturesFlag);

    pscService
        ->createCharacteristic(CommonBleFlags::sensorLocationUuid, NIMBLE_PROPERTY::READ)
        ->setValue(CommonBleFlags::sensorLocationFlag);

    pscService->createCharacteristic(PSCSensorBleFlags::pscControlPointUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE)->setCallbacks(&callbacks);

    return pscService;
}
