#include <string>
#include <vector>

#include "Arduino.h"
#include "ArduinoLog.h"

#include "network.service.h"

using std::string;
using std::to_string;
using std::vector;

NetworkService::NetworkService(EEPROMService &_eepromService) : eepromService(_eepromService), server(Settings::port), webSocket("/ws") {}

void NetworkService::update()
{
    auto now = millis();
    const auto cleanupInterval = 5000;
    if (now - lastCleanupTime > cleanupInterval)
    {
        lastCleanupTime = now;
        webSocket.cleanupClients(2);
    }

    if (!isWifiConnected && WiFiClass::status() == WL_CONNECTED)
    {
        Log.infoln("Connected to the WiFi network");
        Log.infoln("Local ESP32 IP:  %p", WiFi.localIP());

        webSocket.onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                          {
            switch (type)
            {
            case WS_EVT_CONNECT:
                Log.traceln("WebSocket client #%u connected from %p", client->id(), client->remoteIP());
                break;
            case WS_EVT_DISCONNECT:
                Log.traceln("WebSocket client #%u disconnected", client->id());
                break;
            case WS_EVT_DATA:
                handleWebSocketMessage(arg, data, len);
                break;
            case WS_EVT_PONG:
            case WS_EVT_ERROR:
                break;
            } });

        server.addHandler(&webSocket);
        server.begin();
        isWifiConnected = true;
    }
}

void NetworkService::setup()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(Settings::ssid.c_str(), Settings::passphrase.c_str());
    Log.infoln("Connecting to wifi: %s", Settings::ssid.c_str());

    auto connectionTimeout = 20;
    Log.infoln(".");
    while (WiFiClass::status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);

        if (connectionTimeout == 0)
        {
            Serial.print("\n");
            Log.infoln("Was not able to connect to the Wifi retrying in the background");
            break;
        }

        connectionTimeout--;
    }
    Serial.print("\n");
}

void NetworkService::stopServer()
{
    Log.traceln("Stopping web server and Wifi");
    webSocket.closeAll();
    server.end();
    WiFi.disconnect(true);
}

bool NetworkService::isAnyDeviceConnected() const
{
    return isWifiConnected && webSocket.count() > 0;
}

void NetworkService::notifyClients(RowingDataModels::RowingMetrics rowingMetrics, unsigned char batteryLevel, BleServiceFlag bleServiceFlag, ArduinoLogLevel logLevel)
{
    string response;
    response.append("{\"batteryLevel\":" + to_string(batteryLevel));
    response.append(",\"bleServiceFlag\":" + to_string(static_cast<unsigned char>(bleServiceFlag)));
    response.append(",\"logLevel\":" + to_string(static_cast<unsigned char>(logLevel)));
    response.append(",\"revTime\":" + to_string(rowingMetrics.lastRevTime));
    response.append(",\"distance\":" + to_string(rowingMetrics.distance));
    response.append(",\"strokeTime\":" + to_string(rowingMetrics.lastStrokeTime));
    response.append(",\"strokeCount\":" + to_string(rowingMetrics.strokeCount));
    response.append(",\"avgStrokePower\":" + to_string(rowingMetrics.avgStrokePower));
    response.append(",\"driveDuration\":" + to_string(rowingMetrics.driveDuration));
    response.append(",\"recoveryDuration\":" + to_string(rowingMetrics.recoveryDuration));
    response.append(",\"dragFactor\":" + to_string(rowingMetrics.dragCoefficient * 1e6));
    response.append(",\"handleForces\": [");

    for (const auto &handleForce : rowingMetrics.driveHandleForces)
    {
        response.append(to_string(handleForce) + ",");
    }

    if (rowingMetrics.driveHandleForces.size() > 0)
    {
        response.pop_back();
    }
    response.append("]}");

    webSocket.textAll(response.c_str());
}

void NetworkService::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) const
{
    auto *info = static_cast<AwsFrameInfo *>(arg);
    if (static_cast<bool>(info->final) && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        // NOLINTBEGIN
        data[len] = 0;
        string request(reinterpret_cast<char *>(data));
        // NOLINTEND

        if (request.size() < 3)
        {
            Log.traceln("Invalid request: %s", request.c_str());
        }

        auto requestOpCommand = request.substr(1, request.size() - 2);

        Log.infoln("Incoming WS message");

        auto opCommand = parseOpCode(requestOpCommand);

        if (opCommand.size() != 2)
        {
            Log.traceln("Invalid request: %s", request.c_str());
            return;
        }

        Log.infoln("Op Code: %d; Length: %d", opCommand[0], opCommand.size());

        switch (opCommand[0])
        {
        case static_cast<int>(PSCOpCodes::SetLogLevel):
        {
            Log.infoln("Set LogLevel OpCode");

            auto response = PSCResponseOpCodes::InvalidParameter;
            if (opCommand.size() == 2 && opCommand[1] >= 0 && opCommand[1] <= 6)
            {
                Log.infoln("New LogLevel: %d", opCommand[1]);
                eepromService.setLogLevel(static_cast<ArduinoLogLevel>(opCommand[1]));
                response = PSCResponseOpCodes::Successful;

                return;
            }

            Log.infoln("Invalid log level: %d", opCommand[1]);
        }
        break;

        case static_cast<int>(PSCOpCodes::ChangeBleService):
        {
            Log.infoln("Change BLE Service");

            if (opCommand.size() == 2 && opCommand[1] >= 0 && opCommand[1] <= 1)
            {
                Log.infoln("New BLE Service: %s", opCommand[1] == static_cast<unsigned char>(BleServiceFlag::CscService) ? "CSC" : "CPS");
                eepromService.setBleServiceFlag(static_cast<BleServiceFlag>(opCommand[1]));

                Log.verboseln("Restarting device");
                Serial.flush();
                esp_sleep_enable_timer_wakeup(1);
                esp_deep_sleep_start();

                return;
            }

            Log.infoln("Invalid BLE service flag: %d", opCommand[1]);
        }
        break;

        default:
        {
            Log.infoln("Not Supported Op Code: %d", opCommand[0]);
        }
        break;
        }
    }
}

vector<unsigned char> NetworkService::parseOpCode(string requestOpCommand)
{
    auto opCommand = vector<unsigned char>();

    string parsed;
    while (requestOpCommand.empty())
    {
        auto position = requestOpCommand.find(",");
        if (position == string::npos)
        {
            position = requestOpCommand.size();
        }
        parsed = requestOpCommand.substr(0, position);
        requestOpCommand.erase(0, position + 1);
        if (std::any_of(parsed.begin(), parsed.end(), [](unsigned char character)
                        { return !(bool)std::isdigit(character); }))
        {
            Log.traceln("Invalid opCode: %s", parsed.c_str());
            return vector<unsigned char>();
        }
        opCommand.push_back(std::stoi(parsed));
    }
    return opCommand;
}