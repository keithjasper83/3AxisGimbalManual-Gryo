#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include "config.h"

// Global Objects
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");
Adafruit_MPU6050 mpu;
Preferences preferences;

// Servo Objects
Servo servoYaw;
Servo servoPitch;
Servo servoRoll;

// State Variables
int operationMode = MODE_MANUAL;
bool isWiFiConnected = false;
bool isHotspotMode = false;

// Gimbal Position (degrees)
struct GimbalPosition {
    float yaw;
    float pitch;
    float roll;
};

GimbalPosition currentPosition = {90, 90, 90};
GimbalPosition targetPosition = {90, 90, 90};
GimbalPosition autoModeTarget = {90, 90, 90};

// Sensor Data
sensors_event_t accel, gyro, temp;

// PID Control Variables
struct PIDController {
    float kp, ki, kd;
    float prevError;
    float integral;
    unsigned long lastTime;
};

PIDController pidYaw = {KP, KI, KD, 0, 0, 0};
PIDController pidPitch = {KP, KI, KD, 0, 0, 0};
PIDController pidRoll = {KP, KI, KD, 0, 0, 0};

// Timed Move Structure
struct TimedMove {
    unsigned long startTime;
    unsigned long duration;
    GimbalPosition startPos;
    GimbalPosition endPos;
    bool active;
};

TimedMove currentMove = {0, 0, {90, 90, 90}, {90, 90, 90}, false};

// Function Prototypes
void setupWiFi();
void setupHotspot();
void setupWebServer();
void setupSensors();
void setupServos();
void loadConfiguration();
void saveConfiguration();
void updateSensors();
void updateServos();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void sendWebSocketUpdate();
float calculatePID(PIDController &pid, float error, float dt);
void updateAutoMode();
void updateTimedMove();

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== ESP32 3-Axis Gimbal System ===");
    
    // Initialize preferences
    preferences.begin("gimbal", false);
    loadConfiguration();
    
    // Setup components
    setupServos();
    setupSensors();
    setupWiFi();
    setupWebServer();
    
    Serial.println("System ready!");
}

void loop() {
    static unsigned long lastSensorUpdate = 0;
    static unsigned long lastServoUpdate = 0;
    static unsigned long lastWSUpdate = 0;
    
    unsigned long currentTime = millis();
    
    // Update sensors
    if (currentTime - lastSensorUpdate >= SENSOR_UPDATE_RATE) {
        updateSensors();
        lastSensorUpdate = currentTime;
    }
    
    // Update servos
    if (currentTime - lastServoUpdate >= SERVO_UPDATE_RATE) {
        if (operationMode == MODE_AUTO) {
            updateAutoMode();
        }
        updateTimedMove();
        updateServos();
        lastServoUpdate = currentTime;
    }
    
    // Send WebSocket updates
    if (currentTime - lastWSUpdate >= WEBSOCKET_UPDATE_RATE) {
        sendWebSocketUpdate();
        lastWSUpdate = currentTime;
    }
    
    ws.cleanupClients();
}

void setupWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startAttemptTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
        delay(100);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isWiFiConnected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed. Starting hotspot...");
        setupHotspot();
    }
}

void setupHotspot() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(HOTSPOT_SSID, HOTSPOT_PASSWORD);
    isHotspotMode = true;
    Serial.println("Hotspot started!");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

void setupSensors() {
    Serial.println("Initializing MPU6050...");
    Wire.begin(MPU6050_SDA, MPU6050_SCL);
    
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        return;
    }
    
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    Serial.println("MPU6050 initialized!");
}

void setupServos() {
    Serial.println("Initializing servos...");
    
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    
    servoYaw.setPeriodHertz(50);
    servoPitch.setPeriodHertz(50);
    servoRoll.setPeriodHertz(50);
    
    servoYaw.attach(SERVO_PIN_YAW, 500, 2500);
    servoPitch.attach(SERVO_PIN_PITCH, 500, 2500);
    servoRoll.attach(SERVO_PIN_ROLL, 500, 2500);
    
    // Set to center position
    servoYaw.write(currentPosition.yaw);
    servoPitch.write(currentPosition.pitch);
    servoRoll.write(currentPosition.roll);
    
    Serial.println("Servos initialized!");
}

void setupWebServer() {
    // WebSocket handler
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
    
    // Serve static HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>3-Axis Gimbal Control</title>
    <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="bg-gray-900 text-white">
    <div class="container mx-auto p-4">
        <h1 class="text-4xl font-bold mb-8 text-center">3-Axis Gimbal Control</h1>
        
        <div class="grid grid-cols-1 md:grid-cols-2 gap-6 mb-6">
            <div class="bg-gray-800 p-6 rounded-lg">
                <h2 class="text-2xl font-bold mb-4">Control Mode</h2>
                <div class="flex gap-4">
                    <button onclick="setMode('manual')" id="manualBtn" class="bg-blue-600 hover:bg-blue-700 px-6 py-3 rounded-lg">Manual</button>
                    <button onclick="setMode('auto')" id="autoBtn" class="bg-gray-600 hover:bg-gray-700 px-6 py-3 rounded-lg">Auto</button>
                </div>
            </div>
            
            <div class="bg-gray-800 p-6 rounded-lg">
                <h2 class="text-2xl font-bold mb-4">Status</h2>
                <div id="status" class="text-green-400">Connected</div>
                <div class="text-sm text-gray-400 mt-2">
                    <div>Mode: <span id="modeDisplay">Manual</span></div>
                    <div>WiFi: <span id="wifiStatus">Connected</span></div>
                </div>
            </div>
        </div>
        
        <div class="bg-gray-800 p-6 rounded-lg mb-6">
            <h2 class="text-2xl font-bold mb-4">Manual Control</h2>
            <div class="space-y-4">
                <div>
                    <label class="block mb-2">Yaw: <span id="yawValue">90</span>°</label>
                    <input type="range" id="yawSlider" min="0" max="180" value="90" class="w-full" oninput="updatePosition()">
                </div>
                <div>
                    <label class="block mb-2">Pitch: <span id="pitchValue">90</span>°</label>
                    <input type="range" id="pitchSlider" min="0" max="180" value="90" class="w-full" oninput="updatePosition()">
                </div>
                <div>
                    <label class="block mb-2">Roll: <span id="rollValue">90</span>°</label>
                    <input type="range" id="rollSlider" min="0" max="180" value="90" class="w-full" oninput="updatePosition()">
                </div>
                <button onclick="centerGimbal()" class="bg-green-600 hover:bg-green-700 px-6 py-3 rounded-lg w-full">Center Gimbal</button>
            </div>
        </div>
        
        <div class="bg-gray-800 p-6 rounded-lg mb-6">
            <h2 class="text-2xl font-bold mb-4">Auto Mode Target</h2>
            <div class="space-y-4">
                <div>
                    <label class="block mb-2">Target Yaw: <span id="autoYawValue">90</span>°</label>
                    <input type="range" id="autoYawSlider" min="0" max="180" value="90" class="w-full" oninput="updateAutoTarget()">
                </div>
                <div>
                    <label class="block mb-2">Target Pitch: <span id="autoPitchValue">90</span>°</label>
                    <input type="range" id="autoPitchSlider" min="0" max="180" value="90" class="w-full" oninput="updateAutoTarget()">
                </div>
                <div>
                    <label class="block mb-2">Target Roll: <span id="autoRollValue">90</span>°</label>
                    <input type="range" id="autoRollSlider" min="0" max="180" value="90" class="w-full" oninput="updateAutoTarget()">
                </div>
            </div>
        </div>
        
        <div class="bg-gray-800 p-6 rounded-lg mb-6">
            <h2 class="text-2xl font-bold mb-4">Sensor Data</h2>
            <div class="grid grid-cols-2 gap-4 text-sm">
                <div>
                    <div class="font-semibold">Accelerometer</div>
                    <div>X: <span id="accelX">0.00</span> m/s²</div>
                    <div>Y: <span id="accelY">0.00</span> m/s²</div>
                    <div>Z: <span id="accelZ">0.00</span> m/s²</div>
                </div>
                <div>
                    <div class="font-semibold">Gyroscope</div>
                    <div>X: <span id="gyroX">0.00</span> rad/s</div>
                    <div>Y: <span id="gyroY">0.00</span> rad/s</div>
                    <div>Z: <span id="gyroZ">0.00</span> rad/s</div>
                </div>
            </div>
        </div>
        
        <div class="bg-gray-800 p-6 rounded-lg">
            <h2 class="text-2xl font-bold mb-4">Timed Move</h2>
            <div class="space-y-4">
                <div>
                    <label class="block mb-2">Duration (seconds): <span id="durationValue">5</span></label>
                    <input type="range" id="durationSlider" min="1" max="60" value="5" class="w-full" oninput="document.getElementById('durationValue').textContent = this.value">
                </div>
                <div class="grid grid-cols-3 gap-4">
                    <div>
                        <label class="block mb-2">End Yaw</label>
                        <input type="number" id="endYaw" value="90" min="0" max="180" class="w-full bg-gray-700 px-3 py-2 rounded">
                    </div>
                    <div>
                        <label class="block mb-2">End Pitch</label>
                        <input type="number" id="endPitch" value="90" min="0" max="180" class="w-full bg-gray-700 px-3 py-2 rounded">
                    </div>
                    <div>
                        <label class="block mb-2">End Roll</label>
                        <input type="number" id="endRoll" value="90" min="0" max="180" class="w-full bg-gray-700 px-3 py-2 rounded">
                    </div>
                </div>
                <button onclick="startTimedMove()" class="bg-purple-600 hover:bg-purple-700 px-6 py-3 rounded-lg w-full">Start Timed Move</button>
            </div>
        </div>
    </div>
    
    <script>
        let ws;
        
        function connectWebSocket() {
            ws = new WebSocket('ws://' + window.location.hostname + '/ws');
            
            ws.onopen = () => {
                console.log('WebSocket connected');
                document.getElementById('status').textContent = 'Connected';
                document.getElementById('status').className = 'text-green-400';
            };
            
            ws.onclose = () => {
                console.log('WebSocket disconnected');
                document.getElementById('status').textContent = 'Disconnected';
                document.getElementById('status').className = 'text-red-400';
                setTimeout(connectWebSocket, 2000);
            };
            
            ws.onmessage = (event) => {
                const data = JSON.parse(event.data);
                updateDisplay(data);
            };
        }
        
        function updateDisplay(data) {
            if (data.position) {
                document.getElementById('yawValue').textContent = data.position.yaw.toFixed(1);
                document.getElementById('pitchValue').textContent = data.position.pitch.toFixed(1);
                document.getElementById('rollValue').textContent = data.position.roll.toFixed(1);
            }
            
            if (data.sensors) {
                document.getElementById('accelX').textContent = data.sensors.accel.x.toFixed(2);
                document.getElementById('accelY').textContent = data.sensors.accel.y.toFixed(2);
                document.getElementById('accelZ').textContent = data.sensors.accel.z.toFixed(2);
                document.getElementById('gyroX').textContent = data.sensors.gyro.x.toFixed(2);
                document.getElementById('gyroY').textContent = data.sensors.gyro.y.toFixed(2);
                document.getElementById('gyroZ').textContent = data.sensors.gyro.z.toFixed(2);
            }
            
            if (data.mode !== undefined) {
                document.getElementById('modeDisplay').textContent = data.mode === 0 ? 'Manual' : 'Auto';
                updateModeButtons(data.mode);
            }
        }
        
        function updateModeButtons(mode) {
            if (mode === 0) {
                document.getElementById('manualBtn').className = 'bg-blue-600 hover:bg-blue-700 px-6 py-3 rounded-lg';
                document.getElementById('autoBtn').className = 'bg-gray-600 hover:bg-gray-700 px-6 py-3 rounded-lg';
            } else {
                document.getElementById('manualBtn').className = 'bg-gray-600 hover:bg-gray-700 px-6 py-3 rounded-lg';
                document.getElementById('autoBtn').className = 'bg-blue-600 hover:bg-blue-700 px-6 py-3 rounded-lg';
            }
        }
        
        function setMode(mode) {
            const msg = { cmd: 'setMode', mode: mode === 'manual' ? 0 : 1 };
            ws.send(JSON.stringify(msg));
        }
        
        function updatePosition() {
            const yaw = parseInt(document.getElementById('yawSlider').value);
            const pitch = parseInt(document.getElementById('pitchSlider').value);
            const roll = parseInt(document.getElementById('rollSlider').value);
            
            document.getElementById('yawValue').textContent = yaw;
            document.getElementById('pitchValue').textContent = pitch;
            document.getElementById('rollValue').textContent = roll;
            
            const msg = { cmd: 'setPosition', yaw: yaw, pitch: pitch, roll: roll };
            ws.send(JSON.stringify(msg));
        }
        
        function updateAutoTarget() {
            const yaw = parseInt(document.getElementById('autoYawSlider').value);
            const pitch = parseInt(document.getElementById('autoPitchSlider').value);
            const roll = parseInt(document.getElementById('autoRollSlider').value);
            
            document.getElementById('autoYawValue').textContent = yaw;
            document.getElementById('autoPitchValue').textContent = pitch;
            document.getElementById('autoRollValue').textContent = roll;
            
            const msg = { cmd: 'setAutoTarget', yaw: yaw, pitch: pitch, roll: roll };
            ws.send(JSON.stringify(msg));
        }
        
        function centerGimbal() {
            document.getElementById('yawSlider').value = 90;
            document.getElementById('pitchSlider').value = 90;
            document.getElementById('rollSlider').value = 90;
            updatePosition();
        }
        
        function startTimedMove() {
            const duration = parseInt(document.getElementById('durationSlider').value);
            const endYaw = parseInt(document.getElementById('endYaw').value);
            const endPitch = parseInt(document.getElementById('endPitch').value);
            const endRoll = parseInt(document.getElementById('endRoll').value);
            
            const msg = {
                cmd: 'startTimedMove',
                duration: duration * 1000,
                endYaw: endYaw,
                endPitch: endPitch,
                endRoll: endRoll
            };
            ws.send(JSON.stringify(msg));
        }
        
        connectWebSocket();
    </script>
</body>
</html>
        )rawliteral";
        request->send(200, "text/html", html);
    });
    
    // API endpoints
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<512> doc;
        doc["mode"] = operationMode;
        doc["wifi_connected"] = isWiFiConnected;
        doc["hotspot_mode"] = isHotspotMode;
        doc["position"]["yaw"] = currentPosition.yaw;
        doc["position"]["pitch"] = currentPosition.pitch;
        doc["position"]["roll"] = currentPosition.roll;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/position", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<256> doc;
            deserializeJson(doc, data, len);
            
            if (operationMode == MODE_MANUAL) {
                targetPosition.yaw = doc["yaw"] | currentPosition.yaw;
                targetPosition.pitch = doc["pitch"] | currentPosition.pitch;
                targetPosition.roll = doc["roll"] | currentPosition.roll;
            }
            
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        });
    
    server.begin();
    Serial.println("Web server started!");
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket client #%u connected\n", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    } else if (type == WS_EVT_DATA) {
        handleWebSocketMessage(arg, data, len);
    }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, data);
        
        if (error) {
            Serial.println("JSON parse error");
            return;
        }
        
        const char* cmd = doc["cmd"];
        
        if (strcmp(cmd, "setMode") == 0) {
            operationMode = doc["mode"];
            Serial.printf("Mode changed to: %s\n", operationMode == MODE_MANUAL ? "Manual" : "Auto");
        }
        else if (strcmp(cmd, "setPosition") == 0 && operationMode == MODE_MANUAL) {
            targetPosition.yaw = doc["yaw"];
            targetPosition.pitch = doc["pitch"];
            targetPosition.roll = doc["roll"];
        }
        else if (strcmp(cmd, "setAutoTarget") == 0) {
            autoModeTarget.yaw = doc["yaw"];
            autoModeTarget.pitch = doc["pitch"];
            autoModeTarget.roll = doc["roll"];
        }
        else if (strcmp(cmd, "startTimedMove") == 0) {
            currentMove.active = true;
            currentMove.startTime = millis();
            currentMove.duration = doc["duration"];
            currentMove.startPos = currentPosition;
            currentMove.endPos.yaw = doc["endYaw"];
            currentMove.endPos.pitch = doc["endPitch"];
            currentMove.endPos.roll = doc["endRoll"];
            Serial.println("Timed move started");
        }
    }
}

void sendWebSocketUpdate() {
    StaticJsonDocument<512> doc;
    
    doc["mode"] = operationMode;
    doc["position"]["yaw"] = currentPosition.yaw;
    doc["position"]["pitch"] = currentPosition.pitch;
    doc["position"]["roll"] = currentPosition.roll;
    
    doc["sensors"]["accel"]["x"] = accel.acceleration.x;
    doc["sensors"]["accel"]["y"] = accel.acceleration.y;
    doc["sensors"]["accel"]["z"] = accel.acceleration.z;
    doc["sensors"]["gyro"]["x"] = gyro.gyro.x;
    doc["sensors"]["gyro"]["y"] = gyro.gyro.y;
    doc["sensors"]["gyro"]["z"] = gyro.gyro.z;
    
    String output;
    serializeJson(doc, output);
    ws.textAll(output);
}

void updateSensors() {
    mpu.getEvent(&accel, &gyro, &temp);
}

void updateServos() {
    // Smooth movement
    currentPosition.yaw += (targetPosition.yaw - currentPosition.yaw) * 0.1;
    currentPosition.pitch += (targetPosition.pitch - currentPosition.pitch) * 0.1;
    currentPosition.roll += (targetPosition.roll - currentPosition.roll) * 0.1;
    
    // Constrain to limits
    currentPosition.yaw = constrain(currentPosition.yaw, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    currentPosition.pitch = constrain(currentPosition.pitch, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    currentPosition.roll = constrain(currentPosition.roll, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    
    // Write to servos
    servoYaw.write((int)currentPosition.yaw);
    servoPitch.write((int)currentPosition.pitch);
    servoRoll.write((int)currentPosition.roll);
}

float calculatePID(PIDController &pid, float error, float dt) {
    pid.integral += error * dt;
    float derivative = (error - pid.prevError) / dt;
    pid.prevError = error;
    
    return (pid.kp * error) + (pid.ki * pid.integral) + (pid.kd * derivative);
}

void updateAutoMode() {
    unsigned long currentTime = millis();
    float dt = (currentTime - pidYaw.lastTime) / 1000.0;
    
    if (dt > 0) {
        // Calculate angles from gyro
        float gyroYaw = gyro.gyro.z * dt * 57.2958; // Convert to degrees
        float gyroPitch = gyro.gyro.y * dt * 57.2958;
        float gyroRoll = gyro.gyro.x * dt * 57.2958;
        
        // Calculate errors
        float errorYaw = autoModeTarget.yaw - (currentPosition.yaw + gyroYaw);
        float errorPitch = autoModeTarget.pitch - (currentPosition.pitch + gyroPitch);
        float errorRoll = autoModeTarget.roll - (currentPosition.roll + gyroRoll);
        
        // Apply PID control
        float correctionYaw = calculatePID(pidYaw, errorYaw, dt);
        float correctionPitch = calculatePID(pidPitch, errorPitch, dt);
        float correctionRoll = calculatePID(pidRoll, errorRoll, dt);
        
        // Update target positions
        targetPosition.yaw = currentPosition.yaw + correctionYaw;
        targetPosition.pitch = currentPosition.pitch + correctionPitch;
        targetPosition.roll = currentPosition.roll + correctionRoll;
        
        pidYaw.lastTime = currentTime;
        pidPitch.lastTime = currentTime;
        pidRoll.lastTime = currentTime;
    }
}

void updateTimedMove() {
    if (!currentMove.active) return;
    
    unsigned long elapsed = millis() - currentMove.startTime;
    
    if (elapsed >= currentMove.duration) {
        currentMove.active = false;
        targetPosition = currentMove.endPos;
        Serial.println("Timed move completed");
        return;
    }
    
    // Linear interpolation
    float progress = (float)elapsed / (float)currentMove.duration;
    targetPosition.yaw = currentMove.startPos.yaw + (currentMove.endPos.yaw - currentMove.startPos.yaw) * progress;
    targetPosition.pitch = currentMove.startPos.pitch + (currentMove.endPos.pitch - currentMove.startPos.pitch) * progress;
    targetPosition.roll = currentMove.startPos.roll + (currentMove.endPos.roll - currentMove.startPos.roll) * progress;
}

void loadConfiguration() {
    operationMode = preferences.getInt("mode", MODE_MANUAL);
    currentPosition.yaw = preferences.getFloat("yaw", 90);
    currentPosition.pitch = preferences.getFloat("pitch", 90);
    currentPosition.roll = preferences.getFloat("roll", 90);
    targetPosition = currentPosition;
    
    Serial.println("Configuration loaded");
}

void saveConfiguration() {
    preferences.putInt("mode", operationMode);
    preferences.putFloat("yaw", currentPosition.yaw);
    preferences.putFloat("pitch", currentPosition.pitch);
    preferences.putFloat("roll", currentPosition.roll);
    
    Serial.println("Configuration saved");
}
