#include <string>

#include "Arduino.h"
#include "ArduinoLog.h"

#include "network.service.h"

using std::string;
using std::to_string;

NetworkService::NetworkService() : server(Settings::port), webSocket("/ws") {}

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

        webSocket.onEvent(onEvent);
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

void NetworkService::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
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
    }
}

void NetworkService::handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    // AwsFrameInfo *info = (AwsFrameInfo *)arg;
    // if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    // {
    //     data[len] = 0;
    //     if (strcmp((char *)data, "toggle") == 0)
    //     {
    //         notifyClients();
    //     }
    // }
}