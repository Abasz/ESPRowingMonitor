// NOLINTBEGIN(readability-magic-numbers)
#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/ble-services/base-metrics.service.interface.h"
#include "../../../src/peripherals/bluetooth/callbacks/server.callbacks.h"

using namespace fakeit;

TEST_CASE("ServerCallbacks", "[callbacks]")
{
    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();

    Mock<NimBLEConnInfo> mockConnectionInfo;

    When(Method(mockConnectionInfo, getConnHandle)).AlwaysReturn(0);

    Fake(Method(mockNimBLEAdvertising, start));
    Fake(Method(mockNimBLEAdvertising, stop));

    ServerCallbacks serverCallbacks;

    SECTION("onConnect method should")
    {
        SECTION("restart advertising when connection count is less than 2")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0, 1);

            serverCallbacks.onConnect(&mockNimBLEServer.get(), mockConnectionInfo.get());
            serverCallbacks.onConnect(&mockNimBLEServer.get(), mockConnectionInfo.get());

            Verify(Method(mockNimBLEAdvertising, stop)).Exactly(2);
            Verify(Method(mockNimBLEAdvertising, start)).Exactly(2);
        }

        SECTION("should not restart advertising when connection count is 2 or more")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).AlwaysReturn(2);

            serverCallbacks.onConnect(&mockNimBLEServer.get(), mockConnectionInfo.get());

            Verify(Method(mockNimBLEAdvertising, stop)).Never();
            Verify(Method(mockNimBLEAdvertising, start)).Never();
        }
    }

    SECTION("onDisconnect method should")
    {
        SECTION("decrease connected count")
        {
            const auto expectedConnectedCount = 1U;
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0, 1, expectedConnectedCount);

            serverCallbacks.onConnect(&mockNimBLEServer.get(), mockConnectionInfo.get());
            serverCallbacks.onConnect(&mockNimBLEServer.get(), mockConnectionInfo.get());

            serverCallbacks.onDisconnect(&mockNimBLEServer.get(), mockConnectionInfo.get(), 0);

            REQUIRE(serverCallbacks.getConnectionCount() == expectedConnectedCount);
        }
    }
}
// NOLINTEND(readability-magic-numbers)