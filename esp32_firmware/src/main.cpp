#include <Arduino.h>
#include "Services/ConfigManager.h"
#include "Services/WiFiManager.h"
#include "Services/WebManager.h"
#include "Domain/GimbalController.h"
#include "Infrastructure/SensorManager.h"
#include "config.h"

// Dependencies
ConfigManager configManager;
WiFiManagerService wifiManager(configManager);
SensorManager sensorManager;
GimbalController gimbalController(configManager);
WebManager webManager(configManager, gimbalController, sensorManager);

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== ESP32 3-Axis Gimbal System v1.1 ===");

    // Initialize Config
    if (!configManager.begin()) {
        Serial.println("Config Manager Failed! System cannot continue.");
        while(true) { delay(1000); } // Halt - LittleFS is required for web interface
    }

    // Initialize Sensors
    if (!sensorManager.begin()) {
        Serial.println("Sensor Manager Failed! Auto mode will not work properly.");
        // Continue - manual mode can still work
    }
    
    // Initialize Gimbal Controller (Servos)
    gimbalController.begin();
    
    // Initialize WiFi
    wifiManager.begin();
    
    // Initialize Web Manager
    webManager.begin();

    Serial.println("System Ready!");
}

void loop() {
    unsigned long currentTime = millis();
    static unsigned long lastSensorUpdate = 0;
    static unsigned long lastServoUpdate = 0;
    static unsigned long lastWSUpdate = 0;
    
    wifiManager.handle();
    webManager.handle();
    
    // Update Sensors
    if (currentTime - lastSensorUpdate >= SENSOR_UPDATE_RATE) {
        sensorManager.update();
        lastSensorUpdate = currentTime;
    }
    
    // Update Servos / Control Loop
    if (currentTime - lastServoUpdate >= SERVO_UPDATE_RATE) {
        // Initialize lastServoUpdate on first use to avoid a large initial dt
        float dt;
        if (lastServoUpdate == 0) {
            dt = SERVO_UPDATE_RATE / 1000.0; // Use the expected update rate for first iteration
        } else {
            dt = (currentTime - lastServoUpdate) / 1000.0;
        }

        // Get Gyro Data for PID (Simplified)
        // Note: gyro returns rad/s. Multiply by dt to get delta angle in radians.
        // Convert to degrees for consistency with servo (0-180).
        float gyroYaw = sensorManager.getGyroYaw() * dt * 57.2958;
        float gyroPitch = sensorManager.getGyroPitch() * dt * 57.2958;
        float gyroRoll = sensorManager.getGyroRoll() * dt * 57.2958;

        gimbalController.update(dt, gyroYaw, gyroPitch, gyroRoll);
        lastServoUpdate = currentTime;
    }
    
    // WebSocket Broadcast
    if (currentTime - lastWSUpdate >= WEBSOCKET_UPDATE_RATE) {
        webManager.broadcastStatus();
        lastWSUpdate = currentTime;
    }
}
