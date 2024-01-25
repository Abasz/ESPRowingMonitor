#include <string>

#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

#include "../rower/stroke.model.h"
#include "../utils/EEPROM.service.h"
#include "../utils/configuration.h"
#include "../utils/enums.h"

class NetworkService
{
    EEPROMService &eepromService;

    struct MetricsTaskParameters
    {
        AsyncWebSocket &webSocket;
        RowingDataModels::RowingMetrics rowingMetrics;
    } metricTaskParameters;

    struct SettingsTaskParameters
    {
        AsyncWebSocket &webSocket;
        const EEPROMService &eepromService;
        unsigned char batteryLevel = 0;
    } settingsTaskParameters;

    AsyncWebServer server;
    AsyncWebSocket webSocket;

    bool isDisconnectNotified = true;
    bool isServerStarted = false;
    unsigned long lastCleanupTime = 0UL;
    unsigned char batteryLevel = 0;

    void handleWebSocketMessage(const void *arg, uint8_t *data, size_t len);
    static void broadcastSettingsTask(void *parameters);
    static void broadcastMetricsTask(void *parameters);
    static std::vector<unsigned char> parseOpCode(std::string requestOpCommand);

public:
    explicit NetworkService(EEPROMService &_eepromService);
    void setup();
    void update();
    void stopServer();

    void notifyBatteryLevel(unsigned char newBatteryLevel);
    void notifyClients(const RowingDataModels::RowingMetrics &rowingMetrics);
    bool isAnyDeviceConnected() const;
};