#include <ctime>
#include <string>
#include <vector>

#include "Arduino.h"
#include "ArduinoLog.h"
#include "LittleFS.h"

#include "network.service.h"

using std::string;
using std::to_string;

NetworkService::NetworkService(EEPROMService &_eepromService, SdCardService &_sdCardService) : eepromService(_eepromService), sdCardService(_sdCardService), server(Configurations::port), webSocket("/ws"), metricTaskParameters{webSocket, {}, {}}, settingsTaskParameters{webSocket, _eepromService, _sdCardService, 0} {}

void NetworkService::update()
{
    const auto now = millis();
    const auto cleanupInterval = 5000;
    if (now - lastCleanupTime > cleanupInterval)
    {
        lastCleanupTime = now;
        webSocket.cleanupClients(2);
    }

    if (WiFiClass::status() != WL_CONNECTED)
    {
        return;
    }

    isDisconnectNotified = false;

    if (isServerStarted)
    {
        return;
    }

    Log.infoln("Connected to the WiFi network");
    Log.infoln("Local ESP32 IP:  %p", WiFi.localIP());

    webSocket.onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                      {
            switch (type)
            {
            case WS_EVT_CONNECT:
            {
                Log.traceln("WebSocket client #%u connected from %p", client->id(), client->remoteIP());
                
                const auto stackCoreSize = 2048;
                xTaskCreatePinnedToCore(
                    broadcastSettingsTask,
                    "broadcastSettingsTask",
                    stackCoreSize,
                    &settingsTaskParameters,
                    1,
                    NULL,
                    0);
                break;
            }
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

    if constexpr (Configurations::isWebGUIEnabled)
    {
        if (LittleFS.begin())
        {
            Log.traceln("Serving up static Web GUI page");
            const auto lastModified = LittleFS.open("/www/index.html").getLastWrite();
            string formattedDate = "Thu, 01 Jan 1970 00:00:00 GMT";
            std::strftime(formattedDate.data(), 29, "%a, %d %b %Y %H:%M:%S GMT", std::localtime(&lastModified));
            server.serveStatic("/", LittleFS, "/www/")
                .setLastModified(formattedDate.c_str())
                .setDefaultFile("index.html");
        }
    }
    server.begin();
    isServerStarted = true;
}

void NetworkService::setup()
{
    const auto deviceName = Configurations::deviceName + "-(" + string(eepromService.getBleServiceFlag() == BleServiceFlag::CscService ? "CSC)" : "CPS)");
    WiFi.setHostname(deviceName.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(Configurations::ssid.c_str(), Configurations::passphrase.c_str());
    Log.infoln("Connecting to wifi: %s", Configurations::ssid.c_str());

    auto connectionTimeout = 20;
    Log.info(".");
    while (WiFiClass::status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);

        if (connectionTimeout == 0)
        {
            Serial.print("\n");
            Log.info("Was not able to connect to the Wifi retrying in the background");
            break;
        }

        connectionTimeout--;
    }
    Serial.print("\n");
    WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info)
                 { 
                    if (WiFiClass::status() == WL_CONNECT_FAILED)
                    {
                        WiFi.reconnect();
                    }

                    if (!isDisconnectNotified){
                        Log.infoln("Wifi disconnected, reconnect is %s", WiFi.getAutoReconnect() ? "enabled" : "disabled");
                        isDisconnectNotified = true;
                  } },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
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
    return isServerStarted && webSocket.count() > 0;
}

void NetworkService::notifyBatteryLevel(const unsigned char newBatteryLevel)
{
    settingsTaskParameters.batteryLevel = newBatteryLevel;
    if (!isAnyDeviceConnected())
    {
        return;
    }
    const auto stackCoreSize = 2048;
    xTaskCreatePinnedToCore(
        broadcastSettingsTask,
        "broadcastSettingsTask",
        stackCoreSize,
        &settingsTaskParameters,
        1,
        NULL,
        0);
}

void NetworkService::broadcastMetricsTask(void *parameters)
{
    {
        const auto *const params = static_cast<const NetworkService::MetricsTaskParameters *>(parameters);

        string response;

        const auto emptyResponseSize = 70;
        auto responseSize = emptyResponseSize;
        responseSize += params->rowingMetrics.driveHandleForces.size() * 4;
        if (!params->deltaTimes.empty())
        {
            const auto digitCountOfLast = static_cast<int>(std::log10(params->deltaTimes.back())) + 1;
            responseSize += params->deltaTimes.size() * (digitCountOfLast - 4);
        }
        response.reserve(responseSize);

        response.append("{\"data\":[" + to_string(params->rowingMetrics.lastRevTime));
        response.append("," + to_string(lround(params->rowingMetrics.distance)));
        response.append("," + to_string(params->rowingMetrics.lastStrokeTime));
        response.append("," + to_string(params->rowingMetrics.strokeCount));
        response.append("," + to_string(lround(params->rowingMetrics.avgStrokePower)));
        response.append("," + to_string(params->rowingMetrics.driveDuration));
        response.append("," + to_string(params->rowingMetrics.recoveryDuration));
        response.append("," + to_string(lround(params->rowingMetrics.dragCoefficient * 1e6)));

        response.append(",[");
        if (!params->rowingMetrics.driveHandleForces.empty())
        {
            for (const auto &driveHandleForce : params->rowingMetrics.driveHandleForces)
            {
                const auto wholeNumber = static_cast<unsigned short>(std::abs(driveHandleForce));
                const auto decimalPlaces = 3;
                const auto maxSize = static_cast<unsigned char>(std::log10(wholeNumber)) + 2 + decimalPlaces + (driveHandleForce < 0 ? 1 : 0);
                string buffer(" ", maxSize);
                dtostrf(driveHandleForce, maxSize, decimalPlaces, buffer.data());
                response.append(buffer + ",");
            }
            response.pop_back();
        }

        response.append("],[");

        if (!params->deltaTimes.empty())
        {
            for (const auto &deltaTime : params->deltaTimes)
            {
                const auto maxSize = static_cast<unsigned char>(std::log10(deltaTime)) + 1;
                string buffer(" ", maxSize);
                ultoa(deltaTime, buffer.data(), 10);
                response.append(buffer + ",");
            }
            response.pop_back();
        }

        response.append("]]}");

        params->webSocket.binaryAll(response.data(), response.size());
    }
    // Terminate the task
    vTaskDelete(nullptr);
}

void NetworkService::notifyClients(const RowingDataModels::RowingMetrics &rowingMetrics, const vector<unsigned long> &deltaTimes)
{
    if (!isAnyDeviceConnected())
    {
        return;
    }

    metricTaskParameters.rowingMetrics = rowingMetrics;
    metricTaskParameters.deltaTimes = deltaTimes;

    const auto coreStackSize = 2048U;
    auto stackSize = coreStackSize;
    stackSize += rowingMetrics.driveHandleForces.size() * 4;
    if (!deltaTimes.empty())
    {
        const auto digitCountOfLast = static_cast<unsigned char>(std::log10(deltaTimes.back())) + 1;
        const auto digitCountOfFirst = static_cast<unsigned char>(std::log10(deltaTimes.front())) + 1;

        stackSize += deltaTimes.size() * (digitCountOfLast - 4 + std::abs(digitCountOfLast - digitCountOfFirst));
    }

    // The size of stack depends on the length of the string to be created within the task (that depends on the variable sized metrics). Calculate the potential stack size needed dynamically (with some margins) to avoid the task running out of free heep but not using more than it actually necessary potentially starving other parts of the app (that may cause crash).

    xTaskCreatePinnedToCore(
        broadcastMetricsTask,
        "broadcastMetricsTask",
        stackSize,
        &metricTaskParameters,
        1,
        NULL,
        0);
}

void NetworkService::broadcastSettingsTask(void *parameters)
{
    {
        const auto *const params = static_cast<const NetworkService::SettingsTaskParameters *>(parameters);

        string response;
        const auto stringDataLength = 105;
        response.reserve(stringDataLength);
        response.append("{\"batteryLevel\":" + to_string(params->batteryLevel));
        response.append(",\"bleServiceFlag\":" + to_string(static_cast<unsigned char>(params->eepromService.getBleServiceFlag())));
        response.append(",\"logLevel\":" + to_string(static_cast<unsigned char>(params->eepromService.getLogLevel())));
        response.append(",\"logToWebSocket\":" + (Configurations::enableWebSocketDeltaTimeLogging ? to_string(static_cast<unsigned char>(params->eepromService.getLogToWebsocket())) : "null"));
        response.append(",\"logToSdCard\":" + (Configurations::supportSdCardLogging && params->sdCardService.isLogFileOpen() ? to_string(static_cast<unsigned char>(params->eepromService.getLogToSdCard())) : "null"));

        response.append("}");

        params->webSocket.binaryAll(response.c_str(), response.size());
    }

    vTaskDelete(nullptr);
}

void NetworkService::handleWebSocketMessage(const void *const arg, uint8_t *const data, const size_t len)
{
    const auto *const info = static_cast<const AwsFrameInfo *>(arg);
    if (static_cast<bool>(info->final) && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        // NOLINTBEGIN
        data[len] = 0;
        const string request(reinterpret_cast<char *>(data));
        // NOLINTEND

        if (request.size() < 3)
        {
            Log.traceln("Invalid request: %s", request.c_str());
        }

        const auto requestOpCommand = request.substr(1, request.size() - 2);

        Log.infoln("Incoming WS message");

        const auto opCommand = parseOpCode(requestOpCommand);

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

            if (opCommand.size() == 2 && opCommand[1] >= 0 && opCommand[1] <= 6)
            {
                Log.infoln("New LogLevel: %d", opCommand[1]);
                eepromService.setLogLevel(static_cast<ArduinoLogLevel>(opCommand[1]));

                const auto stackCoreSize = 2048;
                xTaskCreatePinnedToCore(
                    broadcastSettingsTask,
                    "broadcastSettingsTask",
                    stackCoreSize,
                    &settingsTaskParameters,
                    1,
                    NULL,
                    0);

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

        case static_cast<int>(PSCOpCodes::SetWebSocketDeltaTimeLogging):
        {
            Log.infoln("Change WebSocket Logging");

            if (opCommand.size() == 2 && opCommand[1] >= 0 && opCommand[1] <= 1)
            {
                Log.infoln("%s WebSocket deltaTime logging", opCommand[1] == static_cast<bool>(true) ? "Enable" : "Disable");
                eepromService.setLogToWebsocket(static_cast<bool>(opCommand[1]));

                const auto stackCoreSize = 2048;
                xTaskCreatePinnedToCore(
                    broadcastSettingsTask,
                    "broadcastSettingsTask",
                    stackCoreSize,
                    &settingsTaskParameters,
                    1,
                    NULL,
                    0);

                return;
            }

            Log.infoln("Invalid OP command for setting WebSocket deltaTime logging, this should be a bool: %d", opCommand[1]);
        }
        break;

        case static_cast<int>(PSCOpCodes::SetSdCardLogging):
        {
            Log.infoln("Change Sd Card Logging");

            if (opCommand.size() == 2 && opCommand[1] >= 0 && opCommand[1] <= 1)
            {
                Log.infoln("%s SdCard logging", opCommand[1] == static_cast<bool>(true) ? "Enable" : "Disable");
                eepromService.setLogToSdCard(static_cast<bool>(opCommand[1]));

                const auto stackCoreSize = 2048;
                xTaskCreatePinnedToCore(
                    broadcastSettingsTask,
                    "broadcastSettingsTask",
                    stackCoreSize,
                    &settingsTaskParameters,
                    1,
                    NULL,
                    0);

                return;
            }

            Log.infoln("Invalid OP command for setting SD Card deltaTime logging, this should be a bool: %d", opCommand[1]);
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
    while (!requestOpCommand.empty())
    {
        auto position = requestOpCommand.find(",");
        if (position == string::npos)
        {
            position = requestOpCommand.size();
        }
        parsed = requestOpCommand.substr(0, position);
        requestOpCommand.erase(0, position + 1);
        if (std::any_of(begin(parsed), end(parsed), [](unsigned char character)
                        { return !(bool)std::isdigit(character); }))
        {
            Log.traceln("Invalid opCode: %s", parsed.c_str());
            return vector<unsigned char>();
        }
        opCommand.push_back(std::stoi(parsed));
    }
    return opCommand;
}