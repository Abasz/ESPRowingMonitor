#include "NimBLEDevice.h"

#include "bluetooth.service.h"

using std::array;

BluetoothService::ServerCallbacks::ServerCallbacks() {}

BluetoothService::ControlPointCallbacks::ControlPointCallbacks(BluetoothService &_bleService) : bleService(_bleService) {}

BluetoothService::HandleForcesCallbacks::HandleForcesCallbacks(BluetoothService &_bleService) : bleService(_bleService) {}

void BluetoothService::ServerCallbacks::onConnect(NimBLEServer *pServer)
{
    if (NimBLEDevice::getServer()->getConnectedCount() < 2)
    {
        Log.verboseln("Device connected");
        NimBLEDevice::getAdvertising()->start();
    }
}

void BluetoothService::HandleForcesCallbacks::onSubscribe(NimBLECharacteristic *const pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue)
{
    if (pCharacteristic->getUUID().toString() == CommonBleFlags::handleForcesUuid)
    {
        bleService.handleForcesClientId = desc->conn_handle;
    }
}

void BluetoothService::ControlPointCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic)
{
    NimBLEAttValue message = pCharacteristic->getValue();

    Log.verboseln("Incoming connection");

    if (message.size() == 0)
    {
        Log.infoln("Invalid request, no Op Code");
        array<unsigned char, 3U> errorResponse = {
            static_cast<unsigned char>(PSCOpCodes::ResponseCode),
            static_cast<unsigned char>(0),
            static_cast<unsigned char>(PSCResponseOpCodes::OperationFailed)};
        pCharacteristic->setValue(errorResponse);
        pCharacteristic->indicate();

        return;
    }

    Log.infoln("Op Code: %d; Length: %d", message[0], message.size());

    switch (message[0])
    {

    case static_cast<unsigned char>(PSCOpCodes::SetLogLevel):
    {
        Log.infoln("Set LogLevel");

        auto response = PSCResponseOpCodes::InvalidParameter;
        if (message.size() == 2 && message[1] >= 0 && message[1] <= 6)
        {
            Log.infoln("New LogLevel: %d", message[1]);
            bleService.eepromService.setLogLevel(static_cast<ArduinoLogLevel>(message[1]));
            response = PSCResponseOpCodes::Successful;
        }

        array<unsigned char, 3U>
            temp = {
                static_cast<unsigned char>(PSCOpCodes::ResponseCode),
                static_cast<unsigned char>(message[0]),
                static_cast<unsigned char>(response)};

        pCharacteristic->setValue(temp);
    }
    break;

    case static_cast<unsigned char>(PSCOpCodes::ChangeBleService):
    {
        Log.infoln("Change BLE Service");

        if (message.size() == 2 && message[1] >= 0 && message[1] <= 1)
        {
            Log.infoln("New BLE Service: %s", message[1] == static_cast<unsigned char>(BleServiceFlag::CscService) ? "CSC" : "CPS");
            bleService.eepromService.setBleServiceFlag(static_cast<BleServiceFlag>(message[1]));
            array<unsigned char, 3U> temp = {
                static_cast<unsigned char>(PSCOpCodes::ResponseCode),
                static_cast<unsigned char>(message[0]),
                static_cast<unsigned char>(PSCResponseOpCodes::Successful)};
            pCharacteristic->setValue(temp);
            pCharacteristic->indicate();

            Log.verboseln("Restarting device in 5s");
            delay(100);
            esp_restart();

            break;
        }

        array<unsigned char, 3U> temp = {
            static_cast<unsigned char>(PSCOpCodes::ResponseCode),
            static_cast<unsigned char>(message[0]),
            static_cast<unsigned char>(PSCResponseOpCodes::InvalidParameter)};

        pCharacteristic->setValue(temp);
    }
    break;

    default:
    {
        Log.infoln("Not Supported Op Code: %d", message[0]);
        array<unsigned char, 3U> response = {
            static_cast<unsigned char>(PSCOpCodes::ResponseCode),
            static_cast<unsigned char>(message[0]),
            static_cast<unsigned char>(PSCResponseOpCodes::UnsupportedOpCode)};
        pCharacteristic->setValue(response);
    }
    break;
    }

    Log.verboseln("Send indicate");
    pCharacteristic->indicate();
}