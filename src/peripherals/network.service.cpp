#include <ctime>
#include <string>
#include <vector>

#include "Arduino.h"
#include "ArduinoLog.h"
#include "LittleFS.h"
#include "WiFi.h"

#include "network.service.h"

using std::string;
using std::to_string;

NetworkService::NetworkService(EEPROMService &_eepromService, SdCardService &_sdCardService) : eepromService(_eepromService), sdCardService(_sdCardService), server(), webSocket(), metricTaskParameters{webSocket, {}, {}}, settingsTaskParameters{webSocket, _eepromService, _sdCardService, 0} {}

void NetworkService::update()
{
    const auto now = millis();
    const auto cleanupInterval = 5'000;
    if (now - lastCleanupTime > cleanupInterval)
    {
        lastCleanupTime = now;
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

    if constexpr (Configurations::isWebsocketEnabled)
    {
        if (LittleFS.begin())
        {

            File keyFile = LittleFS.open("/server.key");
            File certFile = LittleFS.open("/server.crt");
            Serial.println("Reading certificate from LittleFS.");

            if (server.listen(443, certFile.readString().c_str(), keyFile.readString().c_str()) != ESP_OK)
            {
                Log.errorln("errors starting https, falling back to http");
                server.listen(80);
            }

            webSocket.onOpen([&](PsychicWebSocketClient *client)
                             { 
                                Serial.printf("[socket] connection #%u connected from %s\n", client->socket(), client->remoteIP().toString().c_str()); 
                                        const auto stackCoreSize = 2'048;
                                xTaskCreatePinnedToCore(
                                    broadcastSettingsTask,
                                    "broadcastSettingsTask",
                                    stackCoreSize,
                                    &settingsTaskParameters,
                                    1,
                                    NULL,
                                    0); });

            webSocket.onFrame([&](PsychicWebSocketRequest *request, httpd_ws_frame *frame)
                              {
                                const string payload(reinterpret_cast<char *>(frame->payload));
                                
                                const auto payloadOpCommand = payload.substr(1, payload.size() - 2);

                                Log.infoln("Incoming WS message");

                                const auto opCommand = parseOpCode(payloadOpCommand);

                                if (opCommand.size() != 2)
                                {
                                    Log.traceln("Invalid payload: %s", payload.c_str());
                                    return ESP_FAIL;
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

                                        return ESP_OK;
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

                                        return ESP_OK;
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

                                        const auto stackCoreSize = 2'048;
                                        xTaskCreatePinnedToCore(
                                            broadcastSettingsTask,
                                            "broadcastSettingsTask",
                                            stackCoreSize,
                                            &settingsTaskParameters,
                                            1,
                                            NULL,
                                            0);

                                        return ESP_OK;
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

                                        const auto stackCoreSize = 2'048;
                                        xTaskCreatePinnedToCore(
                                            broadcastSettingsTask,
                                            "broadcastSettingsTask",
                                            stackCoreSize,
                                            &settingsTaskParameters,
                                            1,
                                            NULL,
                                            0);

                                        return ESP_OK;
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
                                
                                return ESP_FAIL; });

            webSocket.onClose([](PsychicWebSocketClient *client)
                              { Serial.printf("[socket] connection #%u closed from %s\n", client->socket(), client->remoteIP().toString().c_str()); });
            server.on("/ws", &webSocket);

            if constexpr (Configurations::isWebGUIEnabled)
            {

                Log.traceln("Serving up static Web GUI page");
                const auto lastModified = LittleFS.open("/www/index.html").getLastWrite();
                string formattedDate = "Thu, 01 Jan 1970 00:00:00 GMT";
                std::strftime(formattedDate.data(), 29, "%a, %d %b %Y %H:%M:%S GMT", std::localtime(&lastModified));
                server.serveStatic("/", LittleFS, "/www/")
                    ->setIsDir(true)
                    .setDefaultFile("index.html")
                    .setLastModified(formattedDate.c_str());
            }
        }
    }
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
        delay(1'000);

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
    server.stop();
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

    const auto stackCoreSize = 2'048;
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

        params->webSocket.sendAll(HTTPD_WS_TYPE_BINARY, response.data(), response.size());
    }
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

    const auto coreStackSize = 2'048U;
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

        params->webSocket.sendAll(HTTPD_WS_TYPE_BINARY, response.c_str(), response.size());
    }

    vTaskDelete(nullptr);
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