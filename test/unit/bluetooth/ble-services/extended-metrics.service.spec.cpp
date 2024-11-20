// NOLINTBEGIN(readability-magic-numbers)
#include <string>
#include <vector>

#include "../../include/catch_amalgamated.hpp"
#include "../../include/fakeit.hpp"

#include "./esp_err.h"

#include "../../include/NimBLEDevice.h"

#include "../include/globals.h"

#include "../../../../src/peripherals/bluetooth/ble-services/extended-metrics.service.h"
#include "../../../../src/utils/configuration.h"
#include "../../../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("ExtendedMetricBleService", "[ble-service]")
{
    mockNimBLEServer.Reset();
    mockGlobals.Reset();

    Mock<NimBLECharacteristic> mockExtendedMetricsCharacteristic;
    Mock<NimBLEService> mockExtendedMetricService;

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockExtendedMetricService.get());

    When(OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))).AlwaysReturn(&mockExtendedMetricsCharacteristic.get());

    Fake(OverloadedMethod(mockExtendedMetricsCharacteristic, setValue, void(const unsigned short)));
    Fake(Method(mockExtendedMetricsCharacteristic, setCallbacks));

    SECTION("setup method should")
    {
        ExtendedMetricBleService extendedMetricBleService;

        SECTION("create extended BLE metrics service")
        {
            extendedMetricBleService.setup(&mockNimBLEServer.get());

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string)).Using(CommonBleFlags::extendedMetricsServiceUuid)).Once();
        }

        SECTION("setup handle force measurement characteristic with correct parameters")
        {
            Mock<NimBLECharacteristic> mockHandleForcesCharacteristic;

            const unsigned int expectedMeasurementProperty = NIMBLE_PROPERTY::NOTIFY;

            When(OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::handleForcesUuid, Any())).AlwaysReturn(&mockHandleForcesCharacteristic.get());
            Fake(Method(mockHandleForcesCharacteristic, setCallbacks));

            extendedMetricBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::handleForcesUuid, expectedMeasurementProperty))
                .Once();
        }

        SECTION("setup delta times characteristic with correct parameters")
        {
            Mock<NimBLECharacteristic> mockDeltaTimesCharacteristic;

            const unsigned int expectedMeasurementProperty = NIMBLE_PROPERTY::NOTIFY;

            When(OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::deltaTimesUuid, Any())).AlwaysReturn(&mockDeltaTimesCharacteristic.get());
            Fake(Method(mockDeltaTimesCharacteristic, setCallbacks));

            extendedMetricBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::deltaTimesUuid, expectedMeasurementProperty))
                .Once();
            Verify(Method(mockDeltaTimesCharacteristic, setCallbacks)).Once();
        }

        SECTION("setup extended metrics characteristic with correct parameters")
        {
            Mock<NimBLECharacteristic> mockExtendedMetricsCharacteristic;

            const unsigned int expectedMeasurementProperty = NIMBLE_PROPERTY::NOTIFY;

            When(OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::extendedMetricsUuid, Any())).AlwaysReturn(&mockExtendedMetricsCharacteristic.get());
            Fake(Method(mockExtendedMetricsCharacteristic, setCallbacks));

            extendedMetricBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::extendedMetricsUuid, expectedMeasurementProperty))
                .Once();
        }
    }

    SECTION("isExtendedMetricsSubscribed method should")
    {
        ExtendedMetricBleService extendedMetricBleService;
        extendedMetricBleService.setup(&mockNimBLEServer.get());

        SECTION("return false when extendedMetricBleService setup() method was not called")
        {
            mockGlobals.ClearInvocationHistory();

            ExtendedMetricBleService extendedMetricBleServiceNoSetup;
            Fake(Method(mockGlobals, abort));

            const auto isSubscribed = extendedMetricBleServiceNoSetup.isExtendedMetricsSubscribed();

            REQUIRE(isSubscribed == false);
        }

        SECTION("true when there are subscribers")
        {
            When(Method(mockExtendedMetricsCharacteristic, getSubscribedCount)).Return(1);

            const auto isSubscribed = extendedMetricBleService.isExtendedMetricsSubscribed();

            REQUIRE(isSubscribed == true);
        }

        SECTION("false when there are no subscribers")
        {
            When(Method(mockExtendedMetricsCharacteristic, getSubscribedCount)).Return(0);

            const auto isSubscribed = extendedMetricBleService.isExtendedMetricsSubscribed();

            REQUIRE(isSubscribed == false);
        }
    }

    SECTION("removeDeltaTimesClient method")
    {
        ExtendedMetricBleService extendedMetricBleService;

        SECTION("should remove clientId if exist in the deltaTimes ID list")
        {
            const auto expectedClientId = 0;
            extendedMetricBleService.addDeltaTimesClientId(expectedClientId);

            extendedMetricBleService.removeDeltaTimesClient(expectedClientId);

            REQUIRE_THAT(extendedMetricBleService.getDeltaTimesClientIds(), Catch::Matchers::IsEmpty());
        }

        SECTION("should not remove clientId if does not exist in the deltaTimes ID list")
        {
            const auto expectedClientId = 0;
            extendedMetricBleService.addDeltaTimesClientId(expectedClientId);

            extendedMetricBleService.removeDeltaTimesClient(1);

            REQUIRE_THAT(extendedMetricBleService.getDeltaTimesClientIds(), Catch::Matchers::SizeIs(1));
        }

        SECTION("should return the number of elements removed")
        {
            const auto expectedClientId = 0;
            extendedMetricBleService.addDeltaTimesClientId(expectedClientId);
            extendedMetricBleService.addDeltaTimesClientId(expectedClientId);

            const auto removeFirst = extendedMetricBleService.removeDeltaTimesClient(0);
            const auto removeSecond = extendedMetricBleService.removeDeltaTimesClient(0);

            REQUIRE(removeFirst == 2);
            REQUIRE(removeSecond == 0);
        }
    }

    SECTION("removeHandleForcesClient method")
    {
        ExtendedMetricBleService extendedMetricBleService;

        SECTION("should remove clientId if exist in the handleForces ID list")
        {
            const auto expectedClientId = 0;
            extendedMetricBleService.addHandleForcesClientId(expectedClientId);

            extendedMetricBleService.removeHandleForcesClient(expectedClientId);

            REQUIRE_THAT(extendedMetricBleService.getHandleForcesClientIds(), Catch::Matchers::IsEmpty());
        }

        SECTION("should not remove clientId if does not exist in handleForces ID list")
        {
            const auto expectedClientId = 0;
            extendedMetricBleService.addHandleForcesClientId(expectedClientId);

            extendedMetricBleService.removeHandleForcesClient(1);

            REQUIRE_THAT(extendedMetricBleService.getHandleForcesClientIds(), Catch::Matchers::SizeIs(1));
        }

        SECTION("should return the number of elements removed")
        {
            const auto expectedClientId = 0;
            extendedMetricBleService.addHandleForcesClientId(expectedClientId);
            extendedMetricBleService.addHandleForcesClientId(expectedClientId);

            const auto removeFirst = extendedMetricBleService.removeHandleForcesClient(0);
            const auto removeSecond = extendedMetricBleService.removeHandleForcesClient(0);

            REQUIRE(removeFirst == 2);
            REQUIRE(removeSecond == 0);
        }
    }

    SECTION("addHandleForcesClientId method should add to handleForces client ID list")
    {
        ExtendedMetricBleService extendedMetricBleService;

        const auto expectedClientId = 0;

        extendedMetricBleService.addHandleForcesClientId(expectedClientId);

        REQUIRE_THAT(extendedMetricBleService.getHandleForcesClientIds(), Catch::Matchers::Equals(std::vector<unsigned char>{expectedClientId}));
    }

    SECTION("addDeltaTimesClientId method should add to deltaTimes client ID list")
    {
        ExtendedMetricBleService extendedMetricBleService;

        const auto expectedClientId = 0;

        extendedMetricBleService.addDeltaTimesClientId(expectedClientId);

        REQUIRE_THAT(extendedMetricBleService.getDeltaTimesClientIds(), Catch::Matchers::Equals(std::vector<unsigned char>{expectedClientId}));
    }

    SECTION("getHandleForcesClientId method should get handleForces client ID list")
    {
        ExtendedMetricBleService extendedMetricBleService;

        const std::vector<unsigned char> expectedClientIds{0, 1};
        std::for_each(cbegin(expectedClientIds), cend(expectedClientIds), [&extendedMetricBleService](unsigned char clientId)
                      { extendedMetricBleService.addHandleForcesClientId(clientId); });

        const auto clientIds = extendedMetricBleService.getHandleForcesClientIds();

        REQUIRE_THAT(clientIds, Catch::Matchers::Equals(expectedClientIds));
    }

    SECTION("getDeltaTimesClientId method should get deltaTimes client ID list")
    {
        ExtendedMetricBleService extendedMetricBleService;

        const std::vector<unsigned char> expectedClientIds{0, 1};
        std::for_each(cbegin(expectedClientIds), cend(expectedClientIds), [&extendedMetricBleService](unsigned char clientId)
                      { extendedMetricBleService.addDeltaTimesClientId(clientId); });

        const auto clientIds = extendedMetricBleService.getDeltaTimesClientIds();

        REQUIRE_THAT(clientIds, Catch::Matchers::Equals(expectedClientIds));
    }

    SECTION("calculateMtu method should")
    {
        ExtendedMetricBleService extendedMetricBleService;

        const std::vector<unsigned char> clientIds{0, 1};

        SECTION("return the mtu for the given client ID list")
        {
            const auto expectedMtu = 99;
            When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(99);

            const auto mtu = extendedMetricBleService.calculateMtu(clientIds);

            REQUIRE(mtu == expectedMtu);
        }

        SECTION("return the lowest MTU")
        {
            const auto expectedMtu = 23U;
            When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMtu, 100);

            const auto mtu = extendedMetricBleService.calculateMtu(clientIds);

            REQUIRE(mtu == expectedMtu);
        }

        SECTION("return 512 as MTU even if device reports higher")
        {
            const auto expectedMtu = 512U;
            When(Method(mockNimBLEServer, getPeerMTU)).Return(1'200, 1'000);

            const auto mtu = extendedMetricBleService.calculateMtu(clientIds);

            REQUIRE(mtu == expectedMtu);
        }

        SECTION("ignore zero MTU when calculating minimum")
        {
            const auto expectedMtu = 23U;
            When(Method(mockNimBLEServer, getPeerMTU)).Return(0, expectedMtu);

            const auto mtu = extendedMetricBleService.calculateMtu(clientIds);

            REQUIRE(mtu == expectedMtu);
        }

        SECTION("return max MTU when clientId list is empty")
        {
            const auto expectedMaxMtu = 512U;

            const auto mtu = extendedMetricBleService.calculateMtu({});

            REQUIRE(mtu == expectedMaxMtu);
        }
    }
}
// NOLINTEND(readability-magic-numbers)