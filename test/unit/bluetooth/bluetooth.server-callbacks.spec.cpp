// NOLINTBEGIN(readability-magic-numbers)
#include "../include/catch_amalgamated.hpp"
#include "../include/fakeit.hpp"

#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/ble-services/base-metrics.service.interface.h"
#include "../../../src/peripherals/bluetooth/callbacks/server.callbacks.h"

using namespace fakeit;

TEST_CASE("ServerCallbacks", "[callbacks]")
{
    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();

    Mock<IExtendedMetricBleService> mockExtendedMetricBleService;

    Fake(Method(mockNimBLEAdvertising, start));

    ServerCallbacks serverCallbacks(mockExtendedMetricBleService.get());

    SECTION("onConnect method should")
    {
        SECTION("restart advertising when connection count is less than 2")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).Return(0, 1);

            serverCallbacks.onConnect(&mockNimBLEServer.get());
            serverCallbacks.onConnect(&mockNimBLEServer.get());

            Verify(Method(mockNimBLEAdvertising, start)).Exactly(2);
        }

        SECTION("should not restart advertising when connection count is 2 or more")
        {
            When(Method(mockNimBLEServer, getConnectedCount)).AlwaysReturn(2);

            serverCallbacks.onConnect(&mockNimBLEServer.get());

            Verify(Method(mockNimBLEAdvertising, start)).Never();
        }
    }

    SECTION("onDisconnect method should")
    {
        ble_gap_conn_desc first = {0};
        Fake(Method(mockExtendedMetricBleService, removeDeltaTimesClient));
        Fake(Method(mockExtendedMetricBleService, removeHandleForcesClient));

        SECTION("should remove clientID in the deltaTimes ID list")
        {

            serverCallbacks.onDisconnect(&mockNimBLEServer.get(), &first);

            Verify(Method(mockExtendedMetricBleService, removeHandleForcesClient).Using(first.conn_handle)).Once();
        }

        SECTION("should remove clientID in thehandleForces ID list")
        {
            serverCallbacks.onDisconnect(&mockNimBLEServer.get(), &first);

            Verify(Method(mockExtendedMetricBleService, removeHandleForcesClient).Using(first.conn_handle)).Once();
        }
    }
}
// NOLINTEND(readability-magic-numbers)