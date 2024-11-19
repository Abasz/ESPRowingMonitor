
#include <algorithm>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "./server.callbacks.h"

ServerCallbacks::ServerCallbacks(IExtendedMetricBleService &_extendedMetricsBleService) : extendedMetricsBleService(_extendedMetricsBleService)
{
}

void ServerCallbacks::onConnect(NimBLEServer *pServer)
{
    if (pServer->getConnectedCount() < 2)
    {
        Log.verboseln("Device connected");
        NimBLEDevice::getAdvertising()->start();
    }
}

void ServerCallbacks::onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
{
    Log.verboseln("disconnected ID: %n", desc->conn_handle);

    extendedMetricsBleService.removeHandleForcesClient(desc->conn_handle);
    extendedMetricsBleService.removeDeltaTimesClient(desc->conn_handle);
}