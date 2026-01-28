#include "WebManager.h"

WebManager::WebManager(ConfigManager& configManager, GimbalController& gimbalController, SensorManager& sensorManager)
    : _configManager(configManager),
      _gimbalController(gimbalController),
      _sensorManager(sensorManager),
      _server(HTTP_PORT),
      _ws("/ws")
{}

void WebManager::begin() {
    _ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        this->onWebSocketEvent(server, client, type, arg, data, len);
    });
    _server.addHandler(&_ws);

    // Serve static files
    _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // API Endpoints
    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AppConfig config = _configManager.getConfig();
        StaticJsonDocument<1024> doc;
        doc["wifi_ssid"] = config.wifi_ssid;
        // doc["wifi_password"] = config.wifi_password; // Security risk to send back password
        doc["hotspot_ssid"] = config.hotspot_ssid;
        doc["hotspot_password"] = config.hotspot_password;
        doc["mode"] = config.mode;
        doc["kp"] = config.kp;
        doc["ki"] = config.ki;
        doc["kd"] = config.kd;
        doc["yaw_offset"] = config.yaw_offset;
        doc["pitch_offset"] = config.pitch_offset;
        doc["roll_offset"] = config.roll_offset;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    _server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<1024> doc;
            deserializeJson(doc, data, len);

            AppConfig config = _configManager.getConfig();
            if(doc.containsKey("wifi_ssid")) config.wifi_ssid = doc["wifi_ssid"].as<String>();
            if(doc.containsKey("wifi_password")) config.wifi_password = doc["wifi_password"].as<String>();
            if(doc.containsKey("hotspot_ssid")) config.hotspot_ssid = doc["hotspot_ssid"].as<String>();
            if(doc.containsKey("hotspot_password")) config.hotspot_password = doc["hotspot_password"].as<String>();

            if(doc.containsKey("kp")) config.kp = doc["kp"];
            if(doc.containsKey("ki")) config.ki = doc["ki"];
            if(doc.containsKey("kd")) config.kd = doc["kd"];

            if(doc.containsKey("yaw_offset")) config.yaw_offset = doc["yaw_offset"];
            if(doc.containsKey("pitch_offset")) config.pitch_offset = doc["pitch_offset"];
            if(doc.containsKey("roll_offset")) config.roll_offset = doc["roll_offset"];

            _configManager.updateConfig(config);
            request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    // Version Endpoint
    _server.on("/api/version", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"firmware\": \"1.1.0\", \"hardware\": \"ESP32-GIMBAL-V1\"}");
    });

    _server.begin();
}

void WebManager::handle() {
    _ws.cleanupClients();
}

void WebManager::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        handleWebSocketMessage(arg, data, len);
    }
}

void WebManager::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        StaticJsonDocument<512> doc;
        deserializeJson(doc, data);

        const char* cmd = doc["cmd"];
        if (strcmp(cmd, "setPosition") == 0) {
            _gimbalController.setManualPosition(doc["yaw"], doc["pitch"], doc["roll"]);
        } else if (strcmp(cmd, "setMode") == 0) {
            _gimbalController.setMode(doc["mode"]);
        } else if (strcmp(cmd, "startTimedMove") == 0) {
             GimbalPosition endPos;
             endPos.yaw = doc["endYaw"];
             endPos.pitch = doc["endPitch"];
             endPos.roll = doc["endRoll"];
             _gimbalController.startTimedMove(doc["duration"], endPos);
        } else if (strcmp(cmd, "setAutoTarget") == 0) {
            _gimbalController.setAutoTarget(doc["yaw"], doc["pitch"], doc["roll"]);
        } else if (strcmp(cmd, "center") == 0) {
            _gimbalController.center();
        }
    }
}

void WebManager::broadcastStatus() {
    StaticJsonDocument<1024> doc;
    GimbalPosition pos = _gimbalController.getCurrentPosition();
    SensorData sensors = _sensorManager.getData();

    doc["mode"] = _gimbalController.getMode();
    doc["position"]["yaw"] = pos.yaw;
    doc["position"]["pitch"] = pos.pitch;
    doc["position"]["roll"] = pos.roll;

    doc["sensors"]["accel"]["x"] = sensors.accelX;
    doc["sensors"]["accel"]["y"] = sensors.accelY;
    doc["sensors"]["accel"]["z"] = sensors.accelZ;
    doc["sensors"]["gyro"]["x"] = sensors.gyroX;
    doc["sensors"]["gyro"]["y"] = sensors.gyroY;
    doc["sensors"]["gyro"]["z"] = sensors.gyroZ;

    String output;
    serializeJson(doc, output);
    _ws.textAll(output);
}
