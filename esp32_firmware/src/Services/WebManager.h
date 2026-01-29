#pragma once
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "ConfigManager.h"
#include "../Domain/GimbalController.h"
#include "../Infrastructure/SensorManager.h"

class WebManager {
public:
    WebManager(ConfigManager& configManager, GimbalController& gimbalController, SensorManager& sensorManager);
    void begin();
    void handle();
    void broadcastStatus();

private:
    ConfigManager& _configManager;
    GimbalController& _gimbalController;
    SensorManager& _sensorManager;
    AsyncWebServer _server;
    AsyncWebSocket _ws;

    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
};
