// NOLINTBEGIN(readability-magic-numbers)
#include "../include/catch_amalgamated.hpp"
#include "../include/fakeit.hpp"

#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/ble-services/extended-metrics.service.interface.h"
#include "../../../src/peripherals/bluetooth/callbacks/chunked-notify.callbacks.h"
#include "../../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("ChunkedNotifyMetricCallbacks onSubscribed method", "[callbacks]")
{
    mockNimBLECharacteristic.Reset();

    Mock<IExtendedMetricBleService> mockExtendedMetricBleService;

    ble_gap_conn_desc first = {0};

    Fake(Method(mockExtendedMetricBleService, addHandleForcesClientId));
    Fake(Method(mockExtendedMetricBleService, addDeltaTimesClientId));

    When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(23);

    ChunkedNotifyMetricCallbacks chunkedNotifyMetricCallback(mockExtendedMetricBleService.get());

    SECTION("should ignore subscription if its not to delta times characteristic or handle force characteristic")
    {
        When(Method(mockNimBLECharacteristic, getUUID)).AlwaysReturn(NimBLEUUID{CommonBleFlags::extendedMetricsUuid});

        chunkedNotifyMetricCallback.onSubscribe(&mockNimBLECharacteristic.get(), &first, 0);

        Verify(Method(mockExtendedMetricBleService, addHandleForcesClientId)).Never();
        Verify(Method(mockExtendedMetricBleService, addDeltaTimesClientId)).Never();
    }

    SECTION("should add new connection's client ID")
    {
        SECTION("to deltaTime client ID list when connection is made to delta times")
        {
            When(Method(mockNimBLECharacteristic, getUUID)).AlwaysReturn(NimBLEUUID{CommonBleFlags::deltaTimesUuid});

            chunkedNotifyMetricCallback.onSubscribe(&mockNimBLECharacteristic.get(), &first, 0);

            Verify(Method(mockExtendedMetricBleService, addHandleForcesClientId)).Never();
            Verify(Method(mockExtendedMetricBleService, addDeltaTimesClientId).Using(first.conn_handle)).Once();
        }

        SECTION("to handleForces client ID list when connection is made to handle forces")
        {
            When(Method(mockNimBLECharacteristic, getUUID)).AlwaysReturn(NimBLEUUID{CommonBleFlags::handleForcesUuid});

            chunkedNotifyMetricCallback.onSubscribe(&mockNimBLECharacteristic.get(), &first, 0);

            Verify(Method(mockExtendedMetricBleService, addHandleForcesClientId).Using(first.conn_handle)).Once();
            Verify(Method(mockExtendedMetricBleService, addDeltaTimesClientId)).Never();
        }
    }
}
// NOLINTEND(readability-magic-numbers)