// NOLINTBEGIN(readability-magic-numbers)
#include <array>

#include "../../include/catch_amalgamated.hpp"
#include "../../include/fakeit.hpp"

#include "./esp_err.h"

#include "../../include/Arduino.h"
#include "../../include/NimBLEDevice.h"

#include "../../../../src/peripherals/bluetooth/ble-metrics.model.h"
#include "../../../../src/peripherals/bluetooth/ble-services/base-metrics.service.h"
#include "../../../../src/peripherals/bluetooth/ble-services/settings.service.interface.h"
#include "../../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../../src/utils/configuration.h"
#include "../../../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("BaseMetricsBleService", "[ble-service]")
{
    mockNimBLEServer.Reset();
    mockGlobals.Reset();
    mockArduino.Reset();

    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISettingsBleService> mockMockSettingsBleService;
    Mock<NimBLECharacteristic> mockBaseMetricsCharacteristic;
    Mock<NimBLEService> mockBaseMetricService;

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short))).AlwaysReturn(&mockBaseMetricService.get());

    When(OverloadedMethod(mockBaseMetricService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockBaseMetricsCharacteristic.get());

    Fake(OverloadedMethod(mockBaseMetricsCharacteristic, setValue, void(const unsigned short)));
    Fake(Method(mockBaseMetricsCharacteristic, setCallbacks));

    SECTION("setup method")
    {
        BaseMetricsBleService baseMetricsBleService(mockMockSettingsBleService.get(), mockEEPROMService.get());

        SECTION("when BleServiceFlag::CpsService is passed should")
        {
            Mock<NimBLEService> mockCpsService;
            Mock<NimBLECharacteristic> mockCpsCharacteristic;

            When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(PSCSensorBleFlags::cyclingPowerSvcUuid)).AlwaysReturn(&mockCpsService.get());
            When(OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockCpsCharacteristic.get());
            Fake(OverloadedMethod(mockCpsCharacteristic, setValue, void(const unsigned short)));
            Fake(Method(mockCpsCharacteristic, setCallbacks));

            SECTION("setup CPS service")
            {
                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CpsService);

                Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (unsigned short)).Using(PSCSensorBleFlags::cyclingPowerSvcUuid)).Once();
            }

            SECTION("setup CPS measurement characteristic with correct parameters")
            {
                const unsigned int expectedMeasurementProperty = NIMBLE_PROPERTY::NOTIFY;

                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CpsService);

                Verify(
                    OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                        .Using(PSCSensorBleFlags::pscMeasurementUuid, expectedMeasurementProperty))
                    .Once();
            }

            SECTION("setup CPS features characteristic with correct parameters")
            {
                const unsigned int expectedFlagsProperty = NIMBLE_PROPERTY::READ;

                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CpsService);

                Verify(
                    OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                        .Using(PSCSensorBleFlags::pscFeatureUuid, expectedFlagsProperty))
                    .Once();
                Verify(OverloadedMethod(mockCpsCharacteristic, setValue, void(const unsigned short)).Using(PSCSensorBleFlags::pscFeaturesFlag)).Once();
            }

            SECTION("setup CPS sensor location characteristic with correct parameters")
            {
                const unsigned int expectedFlagsProperty = NIMBLE_PROPERTY::READ;

                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CpsService);

                Verify(
                    OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                        .Using(CommonBleFlags::sensorLocationUuid, expectedFlagsProperty))
                    .Once();
                Verify(OverloadedMethod(mockCpsCharacteristic, setValue, void(const unsigned short)).Using(CommonBleFlags::sensorLocationFlag)).Once();
            }

            SECTION("setup CPS control point characteristic with correct parameters")
            {
                const unsigned int expectedControlPointProperty = NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE;

                When(Method(mockCpsCharacteristic, setCallbacks)).Do([&mockCpsCharacteristic](NimBLECharacteristicCallbacks *callbacks)
                                                                     { mockCpsCharacteristic.get().callbacks = callbacks; });

                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CpsService);

                Verify(
                    OverloadedMethod(mockCpsService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                        .Using(PSCSensorBleFlags::pscControlPointUuid, expectedControlPointProperty))
                    .Once();

                REQUIRE(mockCpsCharacteristic.get().callbacks != nullptr);
            }

            SECTION("set pscTask to the broadcastTask")
            {
                Fake(Method(mockArduino, xTaskCreatePinnedToCore));
                Fake(Method(mockArduino, vTaskDelete));
                Fake(OverloadedMethod(mockCpsCharacteristic, setValue, void(const std::array<unsigned char, 14U>)));
                Fake(Method(mockCpsCharacteristic, notify));
                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CpsService);

                baseMetricsBleService.broadcastBaseMetrics({1, 1, 1, 1, 1});

                Verify(OverloadedMethod(mockCpsCharacteristic, setValue, void(const std::array<unsigned char, 14U>))).Once();
            }
        }

        SECTION("when BleServiceFlag::CsCService is passed should")
        {
            Mock<NimBLEService> mockCscService;
            Mock<NimBLECharacteristic> mockCscCharacteristic;

            When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid)).AlwaysReturn(&mockCscService.get());
            When(OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockCscCharacteristic.get());
            Fake(OverloadedMethod(mockCscCharacteristic, setValue, void(const unsigned short)));
            Fake(Method(mockCscCharacteristic, setCallbacks));

            SECTION("setup CSC service")
            {
                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

                Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (unsigned short)).Using(CSCSensorBleFlags::cyclingSpeedCadenceSvcUuid)).Once();
            }

            SECTION("setup CSC measurement characteristic with correct parameters")
            {
                const unsigned int expectedMeasurementProperty = NIMBLE_PROPERTY::NOTIFY;

                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

                Verify(
                    OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                        .Using(CSCSensorBleFlags::cscMeasurementUuid, expectedMeasurementProperty))
                    .Once();
            }

            SECTION("setup CSC features characteristic with correct parameters")
            {
                const unsigned int expectedFlagsProperty = NIMBLE_PROPERTY::READ;

                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

                Verify(
                    OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                        .Using(CSCSensorBleFlags::cscFeatureUuid, expectedFlagsProperty))
                    .Once();
                Verify(OverloadedMethod(mockCscCharacteristic, setValue, void(const unsigned short)).Using(CSCSensorBleFlags::cscFeaturesFlag)).Once();
            }

            SECTION("setup CSC sensor location characteristic with correct parameters")
            {
                const unsigned int expectedFlagsProperty = NIMBLE_PROPERTY::READ;

                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

                Verify(
                    OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                        .Using(CommonBleFlags::sensorLocationUuid, expectedFlagsProperty))
                    .Once();
                Verify(OverloadedMethod(mockCscCharacteristic, setValue, void(const unsigned short)).Using(CommonBleFlags::sensorLocationFlag)).Once();
            }

            SECTION("setup CSC control point characteristic with correct parameters")
            {
                const unsigned int expectedControlPointProperty = NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE;

                When(Method(mockCscCharacteristic, setCallbacks)).Do([&mockCscCharacteristic](NimBLECharacteristicCallbacks *callbacks)
                                                                     { mockCscCharacteristic.get().callbacks = callbacks; });

                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

                Verify(
                    OverloadedMethod(mockCscService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                        .Using(CSCSensorBleFlags::cscControlPointUuid, expectedControlPointProperty))
                    .Once();

                REQUIRE(mockCscCharacteristic.get().callbacks != nullptr);
            }

            SECTION("should return the created settings NimBLEService")
            {
                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

                auto *const service = baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

                REQUIRE(service == &mockCscService.get());
            }

            SECTION("set cscTask to the broadcastTask")
            {
                Fake(Method(mockArduino, xTaskCreatePinnedToCore));
                Fake(Method(mockArduino, vTaskDelete));
                Fake(OverloadedMethod(mockCscCharacteristic, setValue, void(const std::array<unsigned char, 11U>)));
                Fake(Method(mockCscCharacteristic, notify));
                baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

                baseMetricsBleService.broadcastBaseMetrics({1, 1, 1, 1, 1});

                Verify(OverloadedMethod(mockCscCharacteristic, setValue, void(const std::array<unsigned char, 11U>))).Once();
            }
        }
    }

    SECTION("isSubscribed method should")
    {
        SECTION("trigger ESP_ERR_NOT_FOUND if BaseMetricsBleService setup() method was not called")
        {
            BaseMetricsBleService baseMetricsBleServiceNoSetup(mockMockSettingsBleService.get(), mockEEPROMService.get());
            Fake(Method(mockGlobals, abort));

            baseMetricsBleServiceNoSetup.isSubscribed();

            Verify(Method(mockGlobals, abort).Using(ESP_ERR_NOT_FOUND)).Once();
        }

        BaseMetricsBleService baseMetricsBleService(mockMockSettingsBleService.get(), mockEEPROMService.get());
        baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

        SECTION("true when there are subscribers")
        {
            When(Method(mockBaseMetricsCharacteristic, getSubscribedCount)).Return(1);

            const auto isSubscribed = baseMetricsBleService.isSubscribed();

            REQUIRE(isSubscribed == true);
        }

        SECTION("false when there are no subscribers")
        {
            When(Method(mockBaseMetricsCharacteristic, getSubscribedCount)).Return(0);

            const auto isSubscribed = baseMetricsBleService.isSubscribed();

            REQUIRE(isSubscribed == false);
        }
    }

    SECTION("broadcastBaseMetrics method should")
    {
        const auto secInMicroSec = 1e6L;
        BleMetricsModel::BleMetricsData metrics{
            .revTime = 13'110'000ULL,
            .distance = 2120.4325233,
            .strokeTime = 13'310'000ULL,
            .strokeCount = 2'001U,
            .avgStrokePower = 301.1111,
        };
        const auto expectedStackSize = 2'048U;

        BaseMetricsBleService baseMetricsBleService(mockMockSettingsBleService.get(), mockEEPROMService.get());

        Fake(OverloadedMethod(mockBaseMetricsCharacteristic, setValue, void(const std::array<unsigned char, 11U>)));
        Fake(OverloadedMethod(mockBaseMetricsCharacteristic, setValue, void(const std::array<unsigned char, 14U>)));
        Fake(Method(mockBaseMetricsCharacteristic, notify));

        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        SECTION("setup task pinned to core 0 with the correct stack size")
        {
            baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

            baseMetricsBleService.broadcastBaseMetrics(metrics);

            Verify(Method(mockArduino, xTaskCreatePinnedToCore).Using(Ne(nullptr), StrEq("notifyClients"), Eq(expectedStackSize), Ne(nullptr), Eq(1U), Any(), Eq(0))).Once();
        }

        SECTION("notify PSC with the correct binary data")
        {
            const auto expectedRevTime = static_cast<unsigned short>(lroundl((metrics.revTime / secInMicroSec) * 2'048) % USHRT_MAX);
            const auto expectedDistance = static_cast<unsigned int>(lround(metrics.distance));
            const auto expectedStrokeTime = static_cast<unsigned short>(lroundl((metrics.strokeTime / secInMicroSec) * 1'024) % USHRT_MAX);
            const auto expectedAvgStrokePower = static_cast<short>(lround(metrics.avgStrokePower));

            const auto length = 14U;
            std::array<unsigned char, length> expectedData = {
                static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag),
                static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag >> 8),

                static_cast<unsigned char>(expectedAvgStrokePower),
                static_cast<unsigned char>(expectedAvgStrokePower >> 8),

                static_cast<unsigned char>(expectedDistance),
                static_cast<unsigned char>(expectedDistance >> 8),
                static_cast<unsigned char>(expectedDistance >> 16),
                static_cast<unsigned char>(expectedDistance >> 24),
                static_cast<unsigned char>(expectedRevTime),
                static_cast<unsigned char>(expectedRevTime >> 8),

                static_cast<unsigned char>(metrics.strokeCount),
                static_cast<unsigned char>(metrics.strokeCount >> 8),
                static_cast<unsigned char>(expectedStrokeTime),
                static_cast<unsigned char>(expectedStrokeTime >> 8),
            };

            baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CpsService);

            baseMetricsBleService.broadcastBaseMetrics(metrics);

            Verify(OverloadedMethod(mockBaseMetricsCharacteristic, setValue, void(const std::array<unsigned char, length>)).Using(Eq(expectedData))).Once();
            Verify(Method(mockBaseMetricsCharacteristic, notify)).Once();
        }

        SECTION("notify CSC with the correct binary data")
        {
            const auto expectedRevTime = static_cast<unsigned short>(lroundl((metrics.revTime / secInMicroSec) * 1'024) % USHRT_MAX);
            const auto expectedDistance = static_cast<unsigned int>(lround(metrics.distance));
            const auto expectedStrokeTime = static_cast<unsigned short>(lroundl((metrics.strokeTime / secInMicroSec) * 1'024) % USHRT_MAX);

            const auto length = 11U;
            std::array<unsigned char, length> expectedData = {
                CSCSensorBleFlags::cscMeasurementFeaturesFlag,

                static_cast<unsigned char>(expectedDistance),
                static_cast<unsigned char>(expectedDistance >> 8),
                static_cast<unsigned char>(expectedDistance >> 16),
                static_cast<unsigned char>(expectedDistance >> 24),

                static_cast<unsigned char>(expectedRevTime),
                static_cast<unsigned char>(expectedRevTime >> 8),

                static_cast<unsigned char>(metrics.strokeCount),
                static_cast<unsigned char>(metrics.strokeCount >> 8),
                static_cast<unsigned char>(expectedStrokeTime),
                static_cast<unsigned char>(expectedStrokeTime >> 8),
            };

            baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

            baseMetricsBleService.broadcastBaseMetrics(metrics);

            Verify(OverloadedMethod(mockBaseMetricsCharacteristic, setValue, void(const std::array<unsigned char, length>)).Using(Eq(expectedData))).Once();
            Verify(Method(mockBaseMetricsCharacteristic, notify)).Once();
        }

        SECTION("delete task")
        {
            baseMetricsBleService.setup(&mockNimBLEServer.get(), BleServiceFlag::CscService);

            baseMetricsBleService.broadcastBaseMetrics(metrics);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }
    }
}
// NOLINTEND(readability-magic-numbers)