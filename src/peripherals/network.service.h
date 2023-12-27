#include <string>

#include "PsychicHttpsServer.h"
#include <PsychicHttp.h>

#include "../rower/stroke.model.h"
#include "../utils/EEPROM.service.h"
#include "../utils/configuration.h"
#include "../utils/enums.h"
#include "./sd-card.service.h"

using std::vector;

class NetworkService
{
    EEPROMService &eepromService;
    SdCardService &sdCardService;

    struct MetricsTaskParameters
    {
        // AsyncWebSocket &webSocket;
        PsychicWebSocketHandler &webSocket;
        RowingDataModels::RowingMetrics rowingMetrics;
        vector<unsigned long> deltaTimes;
    } metricTaskParameters;

    struct SettingsTaskParameters
    {
        // AsyncWebSocket &webSocket;
        PsychicWebSocketHandler &webSocket;
        const EEPROMService &eepromService;
        const SdCardService &sdCardService;
        unsigned char batteryLevel = 0;
    } settingsTaskParameters;

    PsychicHttpsServer server;
    PsychicWebSocketHandler webSocket;

    bool isDisconnectNotified = true;
    bool isServerStarted = false;
    unsigned long lastCleanupTime = 0UL;
    unsigned char batteryLevel = 0;

    static void broadcastSettingsTask(void *parameters);
    static void broadcastMetricsTask(void *parameters);
    static vector<unsigned char> parseOpCode(std::string requestOpCommand);

public:
    explicit NetworkService(EEPROMService &_eepromService, SdCardService &_sdCardService);
    void setup();
    void update();
    void stopServer();

    void notifyBatteryLevel(unsigned char newBatteryLevel);
    void notifyClients(const RowingDataModels::RowingMetrics &rowingMetrics, const vector<unsigned long> &deltaTimes);
    bool isAnyDeviceConnected() const;
};