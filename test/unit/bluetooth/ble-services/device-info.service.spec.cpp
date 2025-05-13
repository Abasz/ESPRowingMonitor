// NOLINTBEGIN(readability-magic-numbers)
#include <string>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "../../include/NimBLEDevice.h"

#include "../../../../src/peripherals/bluetooth/ble-services/device-info.service.h"
#include "../../../../src/utils/configuration.h"
#include "../../../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("DeviceInfoBleService", "[ble-service]")
{
    mockNimBLEServer.Reset();

    Mock<NimBLECharacteristic> mockDeviceInfoCharacteristic;
    Mock<NimBLEService> mockDeviceInfoService;

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short))).AlwaysReturn(&mockDeviceInfoService.get());

    When(OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockDeviceInfoCharacteristic.get());

    Fake(OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string)));

    DeviceInfoBleService deviceInfoBleService;

    SECTION("setup method should")
    {
        const unsigned int expectedProperty = NIMBLE_PROPERTY::READ;

        SECTION("create device information service")
        {
            deviceInfoBleService.setup(&mockNimBLEServer.get());

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short)).Using(CommonBleFlags::deviceInfoSvcUuid)).Once();
        }

        SECTION("create manufacturer name characteristic and set its value")
        {
            deviceInfoBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::manufacturerNameSvcUuid, expectedProperty))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string))
                    .Using(Configurations::deviceName))
                .Once();
        }

        SECTION("create model number characteristic and set its value")
        {
            deviceInfoBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string))
                    .Using(Configurations::modelNumber))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::modelNumberSvcUuid, expectedProperty))
                .Once();
        }

        SECTION("create serial number characteristic and set its value")
        {
            deviceInfoBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::serialNumberSvcUuid, expectedProperty))
                .Once();
            Verify(
                OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string))
                    .Using(Configurations::serialNumber))
                .Once();
        }

        SECTION("create firmware number characteristic and set its value")
        {
            deviceInfoBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockDeviceInfoService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))
                    .Using(CommonBleFlags::firmwareNumberSvcUuid, expectedProperty))
                .Once();

            Verify(
                OverloadedMethod(mockDeviceInfoCharacteristic, setValue, void(const std::string))
                    .Using(Configurations::firmwareVersion))
                .Once();
        }
    }
}
// NOLINTEND(readability-magic-numbers)