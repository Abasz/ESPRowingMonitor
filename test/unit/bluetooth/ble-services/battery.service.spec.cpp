// NOLINTBEGIN(readability-magic-numbers)
#include "../../include/catch_amalgamated.hpp"
#include "../../include/fakeit.hpp"

#include "../../include/NimBLEDevice.h"

#include "../../../../src/peripherals/bluetooth/ble-services/battery.service.h"
#include "../../../../src/utils/configuration.h"
#include "../../../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("BatteryBleService", "[ble-service]")
{
    mockNimBLEServer.Reset();

    Mock<NimBLECharacteristic> mockBatteryCharacteristic;
    Mock<NimBLEService> mockBatteryService;

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short))).AlwaysReturn(&mockBatteryService.get());

    When(OverloadedMethod(mockBatteryService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockBatteryCharacteristic.get());

    Fake(OverloadedMethod(mockBatteryCharacteristic, setValue, void(const unsigned short)));
    Fake(Method(mockBatteryCharacteristic, notify));

    BatteryBleService batteryBleService;

    SECTION("setup method should")
    {
        const unsigned int expectedNimBLEProperty = NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY;

        SECTION("initialize battery BLE service with correct UUID")
        {
            batteryBleService.setup(&mockNimBLEServer.get());

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(CommonBleFlags::batterySvcUuid)).Once();
        }

        SECTION("start battery BLE characteristic with correct UUID")
        {
            batteryBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockBatteryService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::batteryLevelUuid, expectedNimBLEProperty))
                .Once();
        }

        SECTION("should return the created settings NimBLEService")
        {
            auto *const service = batteryBleService.setup(&mockNimBLEServer.get());

            REQUIRE(service == &mockBatteryService.get());
        }
    }

    SECTION("setBatteryLevel method should set value on battery characteristic")
    {
        batteryBleService.setup(&mockNimBLEServer.get());
        mockBatteryCharacteristic.ClearInvocationHistory();
        const auto expectedBatteryLevel = 40U;

        batteryBleService.setBatteryLevel(expectedBatteryLevel);
    }

    SECTION("broadcastBatteryLevel method should call notify")
    {
        batteryBleService.setup(&mockNimBLEServer.get());
        mockBatteryCharacteristic.ClearInvocationHistory();

        batteryBleService.broadcastBatteryLevel();

        Verify(Method(mockBatteryCharacteristic, notify));
    }
}
// NOLINTEND(readability-magic-numbers)