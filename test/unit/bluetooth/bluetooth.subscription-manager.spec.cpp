// NOLINTBEGIN(readability-magic-numbers)
#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/ble-services/extended-metrics.service.interface.h"
#include "../../../src/peripherals/bluetooth/callbacks/subscription-manager.callbacks.h"
#include "../../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("SubscriptionManagerCallbacks onSubscribed method", "[callbacks]")
{
    mockNimBLECharacteristic.Reset();

    Mock<NimBLEConnInfo> mockConnectionInfo;
    When(Method(mockConnectionInfo, getConnHandle)).AlwaysReturn(0);

    SubscriptionManagerCallbacks chunkedNotifyMetricCallback;

    SECTION("should add new connection's client ID to client ID list when subscribing")
    {
        chunkedNotifyMetricCallback.onSubscribe(&mockNimBLECharacteristic.get(), mockConnectionInfo.get(), 1);

        REQUIRE(chunkedNotifyMetricCallback.getClientIds().size() == 1);
    }

    SECTION("should remove clientID in thehandleForces ID list")
    {
        chunkedNotifyMetricCallback.onSubscribe(&mockNimBLECharacteristic.get(), mockConnectionInfo.get(), 0);

        REQUIRE(chunkedNotifyMetricCallback.getClientIds().size() == 0);
    }
}
// NOLINTEND(readability-magic-numbers)