#include "WebManager.h"
#include "BluetoothManager.h"

WebManager::WebManager(ConfigManager& configManager, GimbalController& gimbalController, SensorManager& sensorManager)
    : _configManager(configManager),
      _gimbalController(gimbalController),
      _sensorManager(sensorManager),
      _bluetoothManager(nullptr),
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
        // Do not expose hotspot password; instead, indicate whether a password is set
        doc["hotspot_password_set"] = !config.hotspot_password.isEmpty();
        doc["mode"] = config.mode;
        doc["kp"] = config.kp;
        doc["ki"] = config.ki;
        doc["kd"] = config.kd;
        doc["yaw_offset"] = config.yaw_offset;
        doc["pitch_offset"] = config.pitch_offset;
        doc["roll_offset"] = config.roll_offset;
        doc["flat_ref_yaw"] = config.flat_ref_yaw;
        doc["flat_ref_pitch"] = config.flat_ref_pitch;
        doc["flat_ref_roll"] = config.flat_ref_roll;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    _server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            // Require the full body to arrive in a single chunk to avoid parsing partial JSON.
            if (index != 0 || len != total) {
                request->send(400, "application/json", "{\"error\":\"Request body must be sent in a single chunk\"}");
                return;
            }

            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }

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
        request->send(200, "application/json", "{\"firmware\": \"1.2.0\", \"hardware\": \"ESP32-GIMBAL-V1\"}");
    });
    
    // Hardware Status Endpoint
    _server.on("/api/hardware-status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        StaticJsonDocument<512> doc;
        doc["sensor_available"] = _sensorManager.isAvailable();
        doc["config_ok"] = true; // If we're here, config is working
        doc["servo_ok"] = true; // Assume servos are OK if system is running
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Set Flat Reference Endpoint
    _server.on("/api/set-flat-reference", HTTP_POST, [this](AsyncWebServerRequest *request) {
        _gimbalController.setFlatReference();
        request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Flat reference set to current position\"}");
    });
    
    // Run Self Test Endpoint
    _server.on("/api/self-test", HTTP_POST, [this](AsyncWebServerRequest *request) {
        _gimbalController.runSelfTest();
        request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Self-test started - check serial console for results\"}");
    });

    _server.begin();
}

void WebManager::setBluetoothManager(BluetoothManager* bluetoothManager) {
    _bluetoothManager = bluetoothManager;
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
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, data, len);
        if (error) {
            // Invalid JSON; ignore this message safely.
            return;
        }

        JsonVariant cmdVar = doc["cmd"];
        if (!cmdVar.is<const char*>()) {
            // Missing or non-string command; ignore this message safely.
            return;
        }

        const char* cmd = cmdVar.as<const char*>();
        if (strcmp(cmd, "setPosition") == 0) {
            if (doc.containsKey("yaw") && doc.containsKey("pitch") && doc.containsKey("roll")) {
                _gimbalController.setManualPosition(doc["yaw"], doc["pitch"], doc["roll"]);
            }
        } else if (strcmp(cmd, "setMode") == 0) {
            if (doc.containsKey("mode")) {
                _gimbalController.setMode(doc["mode"]);
            }
        } else if (strcmp(cmd, "startTimedMove") == 0) {
            if (doc.containsKey("duration") && doc.containsKey("endYaw") && 
                doc.containsKey("endPitch") && doc.containsKey("endRoll")) {
                GimbalPosition endPos;
                endPos.yaw = doc["endYaw"];
                endPos.pitch = doc["endPitch"];
                endPos.roll = doc["endRoll"];
                _gimbalController.startTimedMove(doc["duration"], endPos);
            }
        } else if (strcmp(cmd, "setAutoTarget") == 0) {
            if (doc.containsKey("yaw") && doc.containsKey("pitch") && doc.containsKey("roll")) {
                _gimbalController.setAutoTarget(doc["yaw"], doc["pitch"], doc["roll"]);
            }
        } else if (strcmp(cmd, "center") == 0) {
            _gimbalController.center();
        } else if (strcmp(cmd, "setFlatReference") == 0) {
            _gimbalController.setFlatReference();
        } else if (strcmp(cmd, "runSelfTest") == 0) {
            _gimbalController.runSelfTest();
        } else if (strcmp(cmd, "setPhoneGyro") == 0) {
            // Handle phone gyroscope data
            if (doc.containsKey("alpha") && doc.containsKey("beta") && doc.containsKey("gamma")) {
                // Phone orientation: alpha (z-axis), beta (x-axis), gamma (y-axis)
                float alpha = doc["alpha"];  // 0 to 360
                float beta = doc["beta"];    // -180 to 180
                float gamma = doc["gamma"];  // -90 to 90
                
                // Validate input ranges
                if (alpha < 0.0f || alpha > 360.0f ||
                    beta < -180.0f || beta > 180.0f ||
                    gamma < -90.0f || gamma > 90.0f) {
                    // Invalid values, ignore
                    return;
                }
                
                // Map phone orientation to gimbal position
                // Alpha (compass heading) -> Yaw: 0-360° maps to 0-180°
                // Beta (front-back tilt) -> Pitch: -180 to 180° maps to 0-180°
                // Gamma (left-right tilt) -> Roll: -90 to 90° maps to 0-180°
                float yaw = alpha / 2.0f;
                float pitch = ((beta + 180.0f) / 360.0f) * 180.0f;
                float roll = ((gamma + 90.0f) / 180.0f) * 180.0f;
                
                _gimbalController.setManualPosition(yaw, pitch, roll);
            }
        }
        // Unknown commands are safely ignored
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
    
    doc["hardware"]["sensor_available"] = _sensorManager.isAvailable();
    doc["hardware"]["bluetooth_connected"] = _bluetoothManager ? _bluetoothManager->isConnected() : false;

    String output;
    serializeJson(doc, output);
    _ws.textAll(output);
}
