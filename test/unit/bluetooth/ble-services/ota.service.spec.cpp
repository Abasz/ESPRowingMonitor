// NOLINTBEGIN(readability-magic-numbers)
#include <string>

#include "../../include/catch_amalgamated.hpp"
#include "../../include/fakeit.hpp"

#include "./esp_err.h"

#include "../../include/Arduino.h"
#include "../../include/NimBLEDevice.h"

#include "../../../../src/peripherals/bluetooth/ble-services/ota.service.h"
#include "../../../../src/utils/configuration.h"
#include "../../../../src/utils/enums.h"
#include "../../../../src/utils/ota-updater/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("OtaBleService", "[ble-service]")
{
    mockNimBLEServer.Reset();
    mockArduino.Reset();

    Mock<IOtaUpdaterService> mockOtaUpdaterService;
    Mock<NimBLECharacteristic> mockOtaTxCharacteristic;
    Mock<NimBLECharacteristic> mockOtaRxCharacteristic;
    Mock<NimBLEService> mockOtaService;

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockOtaService.get());

    When(OverloadedMethod(mockOtaService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::otaTxUuid, Any())).AlwaysReturn(&mockOtaTxCharacteristic.get());
    When(OverloadedMethod(mockOtaService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::otaRxUuid, Any())).AlwaysReturn(&mockOtaRxCharacteristic.get());

    Fake(Method(mockOtaRxCharacteristic, setCallbacks));

    OtaBleService otaBleService(mockOtaUpdaterService.get());

    SECTION("setup method should")
    {
        const unsigned int expectedPropertyTx = NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ;
        const unsigned int expectedPropertyRx = NIMBLE_PROPERTY::WRITE;

        SECTION("create over-the-air update service")
        {
            otaBleService.setup(&mockNimBLEServer.get());

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string)).Using(CommonBleFlags::otaServiceUuid)).Once();
        }

        SECTION("create data transfer characteristic and store it")
        {
            otaBleService.setup(&mockNimBLEServer.get());
            auto *const txCharacteristic = otaBleService.getOtaTx();

            Verify(
                OverloadedMethod(mockOtaService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::otaTxUuid, expectedPropertyTx))
                .Once();
            REQUIRE(txCharacteristic == &mockOtaTxCharacteristic.get());
        }

        SECTION("create data receive characteristic and set callbacks")
        {
            otaBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockOtaService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::otaRxUuid, expectedPropertyRx))
                .Once();
            Verify(Method(mockOtaRxCharacteristic, setCallbacks).Using(Ne(nullptr))).Once();
        }
    }

    SECTION("getOtaTx method should")
    {
        SECTION("call abort when called before begin method is called")
        {
            Fake(Method(mockArduino, abort));

            REQUIRE_THROWS(otaBleService.getOtaTx());

            Verify(Method(mockArduino, abort).Using(ESP_ERR_NOT_FOUND)).Once();
        }

        SECTION("return stored transfer characteristic when called after begin method")
        {
            otaBleService.setup(&mockNimBLEServer.get());

            auto *const txCharacteristic = otaBleService.getOtaTx();

            REQUIRE(txCharacteristic == &mockOtaTxCharacteristic.get());
        }
    }
}
// NOLINTEND(readability-magic-numbers)