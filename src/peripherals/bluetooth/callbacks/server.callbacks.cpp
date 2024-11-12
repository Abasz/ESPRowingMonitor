
#include <algorithm>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../bluetooth.controller.h"
#include "./server.callbacks.h"

ServerCallbacks::ServerCallbacks(BluetoothController &_bleController) : bleController(_bleController)
{
}

void ServerCallbacks::onConnect(NimBLEServer *pServer)
{
    if (NimBLEDevice::getServer()->getConnectedCount() < 2)
    {
        Log.verboseln("Device connected");
        NimBLEDevice::getAdvertising()->start();
    }
}

void ServerCallbacks::onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
{
    Log.verboseln("disconnected ID: %n", desc->conn_handle);

    bleController.extendedMetricsBleService.handleForcesParams.clientIds.erase(
        std::remove_if(
            bleController.extendedMetricsBleService.handleForcesParams.clientIds.begin(),
            bleController.extendedMetricsBleService.handleForcesParams.clientIds.end(),
            [&](char connectionId)
            {
                return connectionId == desc->conn_handle;
            }),
        bleController.extendedMetricsBleService.handleForcesParams.clientIds.end());

    bleController.extendedMetricsBleService.deltaTimesParams.clientIds.erase(
        std::remove_if(
            bleController.extendedMetricsBleService.deltaTimesParams.clientIds.begin(),
            bleController.extendedMetricsBleService.deltaTimesParams.clientIds.end(),
            [&](char connectionId)
            {
                return connectionId == desc->conn_handle;
            }),
        bleController.extendedMetricsBleService.deltaTimesParams.clientIds.end());
}