// NOLINTBEGIN(readability-magic-numbers)
#include "../include/catch_amalgamated.hpp"
#include "../include/fakeit.hpp"

#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/ble-services/extended-metrics.service.interface.h"
#include "../../../src/peripherals/bluetooth/callbacks/connection-manager.callbacks.h"
#include "../../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("ConnectionManagerCallbacks onSubscribed method", "[callbacks]")
{
    mockNimBLECharacteristic.Reset();

    ble_gap_conn_desc first = {0};

    ConnectionManagerCallbacks chunkedNotifyMetricCallback;

    SECTION("should add new connection's client ID to client ID list when subscribing")
    {
        chunkedNotifyMetricCallback.onSubscribe(&mockNimBLECharacteristic.get(), &first, 1);

        REQUIRE(chunkedNotifyMetricCallback.getClientIds().size() == 1);
    }

    SECTION("should remove clientID in thehandleForces ID list")
    {
        chunkedNotifyMetricCallback.onSubscribe(&mockNimBLECharacteristic.get(), &first, 0);

        REQUIRE(chunkedNotifyMetricCallback.getClientIds().size() == 0);
    }
}
// NOLINTEND(readability-magic-numbers)