#include <array>
#include <utility>

#include "esp_err.h"

#include "ArduinoLog.h"

#include "./base-metrics.service.h"
#include "./settings.service.interface.h"

using std::array;

BaseMetricsBleService::BaseMetricsBleService(ISettingsBleService &_settingsBleService, IEEPROMService &_eepromService) : callbacks(_settingsBleService, _eepromService)
{
    broadcastTask = [](void *parameters)
    {
        Log.errorln("Base metrics ble service has not been setup, restarting");

        ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);
    };
}

NimBLEService *BaseMetricsBleService::setup(NimBLEServer *server, const BleServiceFlag bleServiceFlag)
{

    switch (bleServiceFlag)
    {
    case BleServiceFlag::CscService:
        broadcastTask = cscTask;

        return setupCscServices(server);

    case BleServiceFlag::CpsService:
        broadcastTask = pscTask;

        return setupPscServices(server);

    case BleServiceFlag::FtmsService:
        broadcastTask = ftmsTask;

        return setupFtmsServices(server);
    }

    std::unreachable();
}

void BaseMetricsBleService::broadcastBaseMetrics(const BleMetricsModel::BleMetricsData &data)
{
    parameters.data = data;

    const auto coreStackSize = 2'048U;

    xTaskCreatePinnedToCore(
        BaseMetricsBleService::broadcastTask,
        "notifyClients",
        coreStackSize,
        &parameters,
        1,
        NULL,
        0);
}

bool BaseMetricsBleService::isSubscribed()
{
    if (parameters.characteristic == nullptr)
    {
        Log.errorln("Base metrics ble service has not been setup, restarting");

        ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);

        return false;
    }

    return parameters.characteristic->getSubscribedCount() > 0;
}

void (*BaseMetricsBleService::broadcastTask)(void *);

void BaseMetricsBleService::cscTask(void *parameters)
{
    {
        const auto *const params = static_cast<const BaseMetricsBleService::BaseMetricsParams *>(parameters);

        const auto secInMicroSec = 1e6L;
        const auto revTime = static_cast<unsigned short>(lroundl((params->data.revTime / secInMicroSec) * 1'024) % USHRT_MAX);
        const auto revCount = static_cast<unsigned int>(lround(params->data.distance));
        const auto strokeTime = static_cast<unsigned short>(lroundl((params->data.strokeTime / secInMicroSec) * 1'024) % USHRT_MAX);

        const auto length = 11U;
        array<unsigned char, length> temp = {
            CSCSensorBleFlags::cscMeasurementFeaturesFlag,

            static_cast<unsigned char>(revCount),
            static_cast<unsigned char>(revCount >> 8),
            static_cast<unsigned char>(revCount >> 16),
            static_cast<unsigned char>(revCount >> 24),

            static_cast<unsigned char>(revTime),
            static_cast<unsigned char>(revTime >> 8),

            static_cast<unsigned char>(params->data.strokeCount),
            static_cast<unsigned char>(params->data.strokeCount >> 8),
            static_cast<unsigned char>(strokeTime),
            static_cast<unsigned char>(strokeTime >> 8),
        };
        params->characteristic->setValue(temp);
        params->characteristic->notify();
    }
    vTaskDelete(NULL);
}

void BaseMetricsBleService::pscTask(void *parameters)
{
    {
        const auto *const params = static_cast<const BaseMetricsBleService::BaseMetricsParams *>(parameters);

        const auto secInMicroSec = 1e6L;
        const auto revTime = static_cast<unsigned short>(lroundl((params->data.revTime / secInMicroSec) * 2'048) % USHRT_MAX);
        const auto revCount = static_cast<unsigned int>(lround(params->data.distance));
        const auto strokeTime = static_cast<unsigned short>(lroundl((params->data.strokeTime / secInMicroSec) * 1'024) % USHRT_MAX);
        const auto avgStrokePower = static_cast<short>(lround(params->data.avgStrokePower));

        const auto length = 14U;
        array<unsigned char, length> temp = {
            static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag),
            static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag >> 8),

            static_cast<unsigned char>(avgStrokePower),
            static_cast<unsigned char>(avgStrokePower >> 8),

            static_cast<unsigned char>(revCount),
            static_cast<unsigned char>(revCount >> 8),
            static_cast<unsigned char>(revCount >> 16),
            static_cast<unsigned char>(revCount >> 24),
            static_cast<unsigned char>(revTime),
            static_cast<unsigned char>(revTime >> 8),

            static_cast<unsigned char>(params->data.strokeCount),
            static_cast<unsigned char>(params->data.strokeCount >> 8),
            static_cast<unsigned char>(strokeTime),
            static_cast<unsigned char>(strokeTime >> 8),
        };

        params->characteristic->setValue(temp);
        params->characteristic->notify();
    }
    vTaskDelete(NULL);
}

void BaseMetricsBleService::ftmsTask(void *parameters)
{
    {
        const auto *const params = static_cast<const BaseMetricsBleService::BaseMetricsParams *>(parameters);

        const auto secInMicroSec = 1e6L;
        const auto dragFactor = static_cast<unsigned char>(lround(params->data.dragCoefficient * 1e6));
        const unsigned char strokeRate = static_cast<unsigned char>(lroundl((params->data.strokeCount - params->data.previousStrokeCount) / ((params->data.strokeTime - params->data.previousStrokeTime) / secInMicroSec / 60U)));
        const auto pace500m = static_cast<unsigned short>(lroundl(500U / (((params->data.distance - params->data.previousDistance) / 100U) / ((params->data.revTime - params->data.previousRevTime) / secInMicroSec))));
        const auto distance = static_cast<unsigned int>(lround(params->data.distance / 100U));
        const auto avgStrokePower = static_cast<short>(lround(params->data.avgStrokePower));

        const auto length = 14U;
        array<unsigned char, length> temp = {
            static_cast<unsigned char>(FTMSSensorBleFlags::ftmsMeasurementFeaturesFlag),
            static_cast<unsigned char>(FTMSSensorBleFlags::ftmsMeasurementFeaturesFlag >> 8),

            // Stroke rate is with a resolution of 0.5. While this works for a rower it will not work for a kayak erg in all cases (as kayak stroke rate can be up to 160spm)
            static_cast<unsigned char>(strokeRate * 2 > UCHAR_MAX ? UCHAR_MAX : strokeRate * 2),
            static_cast<unsigned char>(params->data.strokeCount),
            static_cast<unsigned char>(params->data.strokeCount >> 8),

            static_cast<unsigned char>(distance),
            static_cast<unsigned char>(distance >> 8),
            static_cast<unsigned char>(distance >> 16),

            static_cast<unsigned char>(pace500m),
            static_cast<unsigned char>(pace500m >> 8),

            static_cast<unsigned char>(avgStrokePower),
            static_cast<unsigned char>(avgStrokePower >> 8),

            static_cast<unsigned char>(dragFactor),
            static_cast<unsigned char>(dragFactor >> 8),
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

NimBLEService *BaseMetricsBleService::setupFtmsServices(NimBLEServer *const server)
{
    Log.infoln("Setting up Fitness Machine Profile");

    auto *ftmsService = server->createService(FTMSSensorBleFlags::FtmsSvcUuid);
    parameters.characteristic = ftmsService->createCharacteristic(FTMSSensorBleFlags::rowerDataUuid, NIMBLE_PROPERTY::NOTIFY);

    ftmsService
        ->createCharacteristic(FTMSSensorBleFlags::FtmsFeaturesUuid, NIMBLE_PROPERTY::READ)
        ->setValue(FTMSSensorBleFlags::ftmsFeaturesFlag);

    return ftmsService;
}