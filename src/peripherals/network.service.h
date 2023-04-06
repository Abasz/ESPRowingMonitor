#include <string>

#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

#include "../rower/stroke.model.h"
#include "../settings.h"
#include "../utils/EEPROM.service.h"
#include "../utils/enums.h"

class NetworkService
{
    EEPROMService &eepromService;

    AsyncWebServer server;
    AsyncWebSocket webSocket;

    bool isWifiConnected = false;
    unsigned long lastCleanupTime = 0UL;

    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) const;
    static std::vector<unsigned char> parseOpCode(std::string requestOpCommand);

public:
    explicit NetworkService(EEPROMService &_eepromService);
    static void setup();
    void update();
    void stopServer();

    void notifyClients(RowingDataModels::RowingMetrics rowingMetrics, unsigned char batteryLevel, BleServiceFlag bleServiceFlag, ArduinoLogLevel logLevel);
    bool isAnyDeviceConnected() const;
};