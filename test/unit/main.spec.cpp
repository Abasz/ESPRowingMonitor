#include <functional>
#include <string>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "./include/Arduino.h"
#include "./include/ArduinoLog.h"
#include "./include/esp_ota_ops.h"

#include "./include/main.globals.h"

#include "../../src/peripherals/peripherals.controller.interface.h"
#include "../../src/rower/stroke.controller.interface.h"
#include "../../src/rower/stroke.model.h"
#include "../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../src/utils/configuration.h"
#include "../../src/utils/ota-updater/ota-updater.service.interface.h"
#include "../../src/utils/power-manager/power-manager.controller.interface.h"

using namespace fakeit;
void setup();
void loop();

TEST_CASE("main", "[main]")
{
    esp_partition_t nextPartition{};
    esp_partition_t runningPartition{};
    RowingDataModels::RowingMetrics rowingMetrics{};

    mockArduino.Reset();
    mockSerial.Reset();
    mockOtaOps.Reset();
    MainTestDoubles::peripheralsControllerMock.Reset();
    MainTestDoubles::powerManagerControllerMock.Reset();
    MainTestDoubles::strokeControllerMock.Reset();
    MainTestDoubles::eepromServiceMock.Reset();
    MainTestDoubles::otaUpdaterServiceMock.Reset();

    Fake(Method(mockSerial, begin));
    When(Method(mockSerial, available)).AlwaysReturn(1);

    Fake(Method(MainTestDoubles::eepromServiceMock, setup));
    When(Method(MainTestDoubles::eepromServiceMock, getLogLevel)).AlwaysReturn(ArduinoLogLevel::LogLevelSilent);

    Fake(Method(MainTestDoubles::peripheralsControllerMock, begin));
    Fake(Method(MainTestDoubles::peripheralsControllerMock, notifyBattery));

    Fake(Method(MainTestDoubles::powerManagerControllerMock, begin));
    When(Method(MainTestDoubles::powerManagerControllerMock, getBatteryLevel)).AlwaysReturn(77U);

    Fake(Method(MainTestDoubles::strokeControllerMock, begin));

    When(Method(mockOtaOps, esp_ota_get_next_update_partition))
        .AlwaysDo([&nextPartition](const esp_partition_t *) -> esp_partition_t *
                  { return &nextPartition; });
    When(Method(mockOtaOps, esp_ota_get_running_partition)).AlwaysReturn(&runningPartition);

    SECTION("setup method should")
    {
        SECTION("initialize serial and controllers")
        {
            setup();

            Verify(Method(mockSerial, begin)).Once();
            Verify(Method(MainTestDoubles::eepromServiceMock, setup)).Once();
            Verify(Method(MainTestDoubles::eepromServiceMock, getLogLevel)).Once();
            Verify(Method(MainTestDoubles::peripheralsControllerMock, begin)).Once();
            Verify(Method(MainTestDoubles::powerManagerControllerMock, begin)).Once();
            Verify(Method(MainTestDoubles::strokeControllerMock, begin)).Once();
            Verify(Method(mockOtaOps, esp_ota_get_next_update_partition)).Once();
            Verify(Method(mockOtaOps, esp_ota_get_running_partition)).Once();
            VerifyNoOtherInvocations(MainTestDoubles::eepromServiceMock);
        }

        SECTION("when reporting initial battery level")
        {
            SECTION("query current battery level")
            {
                setup();

                Verify(Method(MainTestDoubles::powerManagerControllerMock, getBatteryLevel)).Once();
            }

            SECTION("notify peripherals with current battery level")
            {
                setup();

                Verify(Method(MainTestDoubles::peripheralsControllerMock, notifyBattery).Using(77U)).Once();
            }
        }
    }

    SECTION("loop method should")
    {
        When(Method(MainTestDoubles::otaUpdaterServiceMock, isUpdating)).AlwaysReturn(false);

        Fake(Method(MainTestDoubles::strokeControllerMock, update));
        When(Method(MainTestDoubles::powerManagerControllerMock, getBatteryLevel)).AlwaysReturn(80U);
        Fake(Method(MainTestDoubles::peripheralsControllerMock, update));
        When(Method(MainTestDoubles::strokeControllerMock, getLastImpulseTime)).AlwaysReturn(123UL);
        When(Method(MainTestDoubles::peripheralsControllerMock, isAnyDeviceConnected)).AlwaysReturn(false);
        Fake(Method(MainTestDoubles::powerManagerControllerMock, update));

        When(Method(MainTestDoubles::powerManagerControllerMock, getPreviousBatteryLevel)).AlwaysReturn(80U);
        Fake(Method(MainTestDoubles::peripheralsControllerMock, notifyBattery));
        Fake(Method(MainTestDoubles::powerManagerControllerMock, setPreviousBatteryLevel));

        SECTION("return early while ota update is active")
        {
            When(Method(MainTestDoubles::otaUpdaterServiceMock, isUpdating)).AlwaysReturn(true);

            loop();

            Verify(Method(MainTestDoubles::otaUpdaterServiceMock, isUpdating)).Once();
            Verify(Method(MainTestDoubles::strokeControllerMock, update)).Never();
            Verify(Method(MainTestDoubles::peripheralsControllerMock, update)).Never();
            Verify(Method(MainTestDoubles::powerManagerControllerMock, update)).Never();
        }

        SECTION("when new stroke data is available")
        {
            When(Method(MainTestDoubles::peripheralsControllerMock, isAnyDeviceConnected)).AlwaysReturn(true);

            When(Method(MainTestDoubles::strokeControllerMock, getRawImpulseCount)).AlwaysReturn(2UL);
            When(Method(MainTestDoubles::strokeControllerMock, getPreviousRawImpulseCount)).AlwaysReturn(1UL);
            When(Method(MainTestDoubles::strokeControllerMock, getDeltaTime)).AlwaysReturn(42UL);
            Fake(Method(MainTestDoubles::strokeControllerMock, setPreviousRawImpulseCount));

            When(Method(MainTestDoubles::strokeControllerMock, getStrokeCount)).AlwaysReturn(2U);
            When(Method(MainTestDoubles::strokeControllerMock, getPreviousStrokeCount)).AlwaysReturn(1U);
            When(Method(MainTestDoubles::strokeControllerMock, getAllData))
                .AlwaysDo([&rowingMetrics]() -> const RowingDataModels::RowingMetrics &
                          { return rowingMetrics; });
            When(Method(mockArduino, millis)).AlwaysReturn(10'000UL);
            Fake(Method(MainTestDoubles::peripheralsControllerMock, updateData));
            When(Method(MainTestDoubles::strokeControllerMock, getDriveDuration)).AlwaysReturn(10U);
            When(Method(MainTestDoubles::strokeControllerMock, getRecoveryDuration)).AlwaysReturn(20U);
            When(Method(MainTestDoubles::strokeControllerMock, getDragFactor)).AlwaysReturn(30U);
            When(Method(MainTestDoubles::strokeControllerMock, getAvgStrokePower)).AlwaysReturn(40);
            When(Method(MainTestDoubles::strokeControllerMock, getDistance)).AlwaysReturn(50.0);
            Fake(Method(MainTestDoubles::strokeControllerMock, setPreviousStrokeCount));

            Fake(Method(MainTestDoubles::peripheralsControllerMock, updateDeltaTime));

            SECTION("propagate updated metrics to peripherals")
            {
                loop();

                Verify(Method(MainTestDoubles::otaUpdaterServiceMock, isUpdating)).Once();
                Verify(Method(MainTestDoubles::strokeControllerMock, update)).Once();
                Verify(Method(MainTestDoubles::peripheralsControllerMock, update).Using(80U)).Once();
                Verify(Method(MainTestDoubles::powerManagerControllerMock, update).Using(123UL, true)).Once();
                Verify(Method(MainTestDoubles::peripheralsControllerMock, updateDeltaTime).Using(42UL)).Once();
                Verify(Method(MainTestDoubles::peripheralsControllerMock, updateData)).Once();
                Verify(Method(MainTestDoubles::peripheralsControllerMock, notifyBattery)).Never();
            }

            SECTION("advance previous impulse and stroke counters")
            {
                loop();

                Verify(Method(MainTestDoubles::strokeControllerMock, setPreviousRawImpulseCount)).Once();
                Verify(Method(MainTestDoubles::strokeControllerMock, setPreviousStrokeCount)).Once();
            }
        }

        SECTION("when stroke count is unchanged")
        {
            SECTION("skip BLE data update when interval is not elapsed")
            {
                When(Method(MainTestDoubles::strokeControllerMock, getRawImpulseCount)).AlwaysReturn(1UL);
                When(Method(MainTestDoubles::strokeControllerMock, getPreviousRawImpulseCount)).AlwaysReturn(1UL);

                // First call changes stroke count to seed lastUpdateTime in main.cpp, second call keeps counts equal.
                When(Method(MainTestDoubles::strokeControllerMock, getStrokeCount)).Return(2U).AlwaysReturn(2U);
                When(Method(MainTestDoubles::strokeControllerMock, getPreviousStrokeCount)).Return(1U).AlwaysReturn(2U);

                When(Method(MainTestDoubles::strokeControllerMock, getAllData))
                    .AlwaysDo([&rowingMetrics]() -> const RowingDataModels::RowingMetrics &
                              { return rowingMetrics; });

                When(Method(mockArduino, millis)).Return(100UL).Return(100UL);

                Fake(Method(MainTestDoubles::peripheralsControllerMock, updateData));
                Fake(Method(MainTestDoubles::strokeControllerMock, setPreviousStrokeCount));

                loop();
                loop();

                Verify(Method(MainTestDoubles::peripheralsControllerMock, updateData)).Once();
                Verify(Method(MainTestDoubles::strokeControllerMock, getAllData)).Once();
                Verify(Method(MainTestDoubles::peripheralsControllerMock, notifyBattery)).Never();
            }

            SECTION("update BLE data when interval is elapsed")
            {
                When(Method(MainTestDoubles::strokeControllerMock, getRawImpulseCount)).AlwaysReturn(1UL);
                When(Method(MainTestDoubles::strokeControllerMock, getPreviousRawImpulseCount)).AlwaysReturn(1UL);

                // First call changes stroke count to seed lastUpdateTime in main.cpp, second call keeps counts equal.
                When(Method(MainTestDoubles::strokeControllerMock, getStrokeCount)).Return(2U).AlwaysReturn(2U);
                When(Method(MainTestDoubles::strokeControllerMock, getPreviousStrokeCount)).Return(1U).AlwaysReturn(2U);
                When(Method(MainTestDoubles::strokeControllerMock, getAllData))
                    .AlwaysDo([&rowingMetrics]() -> const RowingDataModels::RowingMetrics &
                              { return rowingMetrics; });

                When(Method(mockArduino, millis))
                    .Return(100UL)
                    .Return(Configurations::minBleUpdateInterval + 101UL);

                Fake(Method(MainTestDoubles::peripheralsControllerMock, updateData));
                Fake(Method(MainTestDoubles::strokeControllerMock, setPreviousStrokeCount));

                loop();
                loop();

                Verify(Method(MainTestDoubles::peripheralsControllerMock, updateData)).Exactly(2);
                Verify(Method(MainTestDoubles::strokeControllerMock, getAllData)).Exactly(2);
                Verify(Method(MainTestDoubles::peripheralsControllerMock, notifyBattery)).Never();
            }

            SECTION("skip BLE data update when elapsed time equals min interval")
            {
                When(Method(MainTestDoubles::strokeControllerMock, getRawImpulseCount)).AlwaysReturn(1UL);
                When(Method(MainTestDoubles::strokeControllerMock, getPreviousRawImpulseCount)).AlwaysReturn(1UL);

                // First call changes stroke count to seed lastUpdateTime in main.cpp, second call keeps counts equal.
                When(Method(MainTestDoubles::strokeControllerMock, getStrokeCount)).Return(2U).AlwaysReturn(2U);
                When(Method(MainTestDoubles::strokeControllerMock, getPreviousStrokeCount)).Return(1U).AlwaysReturn(2U);
                When(Method(MainTestDoubles::strokeControllerMock, getAllData))
                    .AlwaysDo([&rowingMetrics]() -> const RowingDataModels::RowingMetrics &
                              { return rowingMetrics; });

                When(Method(mockArduino, millis))
                    .Return(100UL)
                    .Return(Configurations::minBleUpdateInterval + 100UL);

                Fake(Method(MainTestDoubles::peripheralsControllerMock, updateData));
                Fake(Method(MainTestDoubles::strokeControllerMock, setPreviousStrokeCount));

                loop();
                loop();

                Verify(Method(MainTestDoubles::peripheralsControllerMock, updateData)).Once();
                Verify(Method(MainTestDoubles::strokeControllerMock, getAllData)).Once();
                Verify(Method(MainTestDoubles::peripheralsControllerMock, notifyBattery)).Never();
            }
        }

        SECTION("when battery value changes")
        {
            When(Method(MainTestDoubles::strokeControllerMock, getRawImpulseCount)).AlwaysReturn(1UL);
            When(Method(MainTestDoubles::strokeControllerMock, getPreviousRawImpulseCount)).AlwaysReturn(1UL);
            When(Method(MainTestDoubles::strokeControllerMock, getStrokeCount)).AlwaysReturn(2U);
            When(Method(MainTestDoubles::strokeControllerMock, getPreviousStrokeCount)).AlwaysReturn(2U);
            When(Method(mockArduino, millis)).AlwaysReturn(100UL);
            When(Method(MainTestDoubles::strokeControllerMock, getAllData))
                .AlwaysDo([&rowingMetrics]() -> const RowingDataModels::RowingMetrics &
                          { return rowingMetrics; });
            Fake(Method(MainTestDoubles::peripheralsControllerMock, updateData));

            When(Method(MainTestDoubles::powerManagerControllerMock, getBatteryLevel)).AlwaysReturn(75U);
            When(Method(MainTestDoubles::powerManagerControllerMock, getPreviousBatteryLevel)).AlwaysReturn(80U);

            SECTION("notify battery with current level")
            {
                loop();

                Verify(Method(MainTestDoubles::peripheralsControllerMock, notifyBattery).Using(75U)).Once();
            }

            SECTION("store current battery as previous level")
            {
                loop();

                Verify(Method(MainTestDoubles::powerManagerControllerMock, setPreviousBatteryLevel)).Once();
            }
        }
    }
}
