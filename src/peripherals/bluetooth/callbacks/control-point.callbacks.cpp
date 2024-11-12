
#include <array>

#include "ArduinoLog.h"
#include "NimBLEDevice.h"

#include "../bluetooth.controller.h"
#include "./control-point.callbacks.h"

using std::array;

ControlPointCallbacks::ControlPointCallbacks(BluetoothController &_bleController) : bleController(_bleController)
{
}

void ControlPointCallbacks::onWrite(NimBLECharacteristic *const pCharacteristic)
{
    NimBLEAttValue message = pCharacteristic->getValue();

    Log.verboseln("Incoming connection");

    if (message.size() == 0)
    {
        Log.infoln("Invalid request, no Op Code");
        array<unsigned char, 3U> errorResponse = {
            static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(0),
            static_cast<unsigned char>(ResponseOpCodes::OperationFailed)};
        pCharacteristic->setValue(errorResponse);
        pCharacteristic->indicate();

        return;
    }

    Log.infoln("Op Code: %d; Length: %d", message[0], message.size());

    switch (message[0])
    {

    case static_cast<unsigned char>(SettingsOpCodes::SetLogLevel):
    {
        Log.infoln("Set LogLevel");

        const auto response = processLogLevel(message, pCharacteristic);

        array<unsigned char, 3U>
            temp = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(message[0]),
                static_cast<unsigned char>(response)};

        pCharacteristic->setValue(temp);
    }
    break;

    case static_cast<unsigned char>(SettingsOpCodes::ChangeBleService):
    {
        Log.infoln("Change BLE Service");

        if (message.size() == 2 && message[1] >= 0 && message[1] <= 1)
        {
            processBleServiceChange(message, pCharacteristic);

            return;
        }

        array<unsigned char, 3U> temp = {
            static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(message[0]),
            static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

        pCharacteristic->setValue(temp);
    }
    break;

    case static_cast<unsigned char>(SettingsOpCodes::SetSdCardLogging):
    {
        Log.infoln("Change Sd Card Logging");

        const auto response = processSdCardLogging(message, pCharacteristic);

        array<unsigned char, 3U> temp = {
            static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(message[0]),
            static_cast<unsigned char>(response)};

        pCharacteristic->setValue(temp);
    }
    break;

    case static_cast<unsigned char>(SettingsOpCodes::SetDeltaTimeLogging):
    {
        Log.infoln("Change deltaTime logging");

        const auto response = processDeltaTimeLogging(message, pCharacteristic);

        array<unsigned char, 3U> temp = {
            static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(message[0]),
            static_cast<unsigned char>(response)};

        pCharacteristic->setValue(temp);
    }
    break;

    default:
    {
        Log.infoln("Not Supported Op Code: %d", message[0]);
        array<unsigned char, 3U> response = {
            static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(message[0]),
            static_cast<unsigned char>(ResponseOpCodes::UnsupportedOpCode)};
        pCharacteristic->setValue(response);
    }
    break;
    }

    Log.verboseln("Send indicate");
    pCharacteristic->indicate();
}

ResponseOpCodes ControlPointCallbacks::processLogLevel(const NimBLEAttValue &message, NimBLECharacteristic *const pCharacteristic)
{
    if (message.size() != 2 || message[1] < 0 || message[1] > 6)
    {
        return ResponseOpCodes::InvalidParameter;
    }

    Log.infoln("New LogLevel: %d", message[1]);
    bleController.eepromService.setLogLevel(static_cast<ArduinoLogLevel>(message[1]));

    bleController.notifySettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processSdCardLogging(const NimBLEAttValue &message, NimBLECharacteristic *const pCharacteristic)
{
    if (message.size() != 2 || message[1] < 0 || message[1] > 1)
    {
        Log.infoln("Invalid OP command for setting SD Card deltaTime logging, this should be a bool: %d", message[1]);

        return ResponseOpCodes::InvalidParameter;
    }

    const auto shouldEnable = static_cast<bool>(message[1]);
    Log.infoln("%s SdCard logging", shouldEnable ? "Enable" : "Disable");
    bleController.eepromService.setLogToSdCard(shouldEnable);

    bleController.notifySettings();

    return ResponseOpCodes::Successful;
}

ResponseOpCodes ControlPointCallbacks::processDeltaTimeLogging(const NimBLEAttValue &message, NimBLECharacteristic *const pCharacteristic)
{
    if (message.size() != 2 || message[1] < 0 || message[1] > 1)
    {
        Log.infoln("Invalid OP command for setting deltaTime logging, this should be a bool: %d", message[1]);

        return ResponseOpCodes::InvalidParameter;
    }

    const auto shouldEnable = static_cast<bool>(message[1]);

    Log.infoln("%s deltaTime logging", shouldEnable ? "Enable" : "Disable");
    bleController.eepromService.setLogToBluetooth(shouldEnable);

    bleController.notifySettings();

    return ResponseOpCodes::Successful;
}

void ControlPointCallbacks::processBleServiceChange(const NimBLEAttValue &message, NimBLECharacteristic *const pCharacteristic)
{
    Log.infoln("New BLE Service: %s", message[1] == static_cast<unsigned char>(BleServiceFlag::CscService) ? "CSC" : "CPS");
    bleController.eepromService.setBleServiceFlag(static_cast<BleServiceFlag>(message[1]));
    array<unsigned char, 3U> temp = {
        static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
        static_cast<unsigned char>(message[0]),
        static_cast<unsigned char>(ResponseOpCodes::Successful)};
    pCharacteristic->setValue(temp);
    pCharacteristic->indicate();

    bleController.notifySettings();

    Log.verboseln("Restarting device in 5s");
    delay(100);
    esp_restart();
}