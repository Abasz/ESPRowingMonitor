#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

#include "../rower/stroke.model.h"
#include "../settings.h"
#include "../utils/enums.h"

class NetworkService
{
    AsyncWebServer server;
    AsyncWebSocket webSocket;

    bool isWifiConnected = false;
    unsigned long lastCleanupTime = 0UL;

    static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    static void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

public:
    NetworkService();
    static void setup();
    void update();
    void stopServer();

    void notifyClients(RowingDataModels::RowingMetrics rowingMetrics, unsigned char batteryLevel, BleServiceFlag bleServiceFlag, ArduinoLogLevel logLevel);
    bool isAnyDeviceConnected() const;
};