#include <Arduino.h>
#include "Services/ConfigManager.h"
#include "Services/WiFiManager.h"
#include "Services/WebManager.h"
#include "Services/BluetoothManager.h"
#include "Domain/GimbalController.h"
#include "Infrastructure/SensorManager.h"
#include "config.h"

// Dependencies
ConfigManager configManager;
WiFiManagerService wifiManager(configManager);
SensorManager sensorManager;
GimbalController gimbalController(configManager);
WebManager webManager(configManager, gimbalController, sensorManager);
BluetoothManager bluetoothManager(gimbalController);

// Button state tracking
unsigned long buttonPressStart = 0;
bool buttonPressed = false;
bool longPressHandled = false;

// Hardware status
struct HardwareStatus {
    bool sensorAvailable;
    bool configOk;
} hwStatus;

void powerOnSelfTest() {
    Serial.println("\n=== Power-On Self Test (POST) ===");
    
    // Test 1: Config System
    Serial.print("Config System: ");
    hwStatus.configOk = configManager.begin();
    Serial.println(hwStatus.configOk ? "OK" : "FAILED");
    
    // Test 2: Sensor System
    Serial.print("MPU6050 Sensor: ");
    hwStatus.sensorAvailable = sensorManager.begin();
    Serial.println(hwStatus.sensorAvailable ? "OK" : "FAILED (Manual mode only)");
    
    // Test 3: Servo System
    Serial.print("Servo Controllers: ");
    gimbalController.begin();
    Serial.println("OK");
    
    Serial.println("=================================\n");
}

void handleButton() {
    int buttonState = digitalRead(BUTTON_PIN);
    unsigned long currentTime = millis();
    
    if (buttonState == LOW) { // Button pressed (assuming active-low with pull-up)
        if (!buttonPressed) {
            // Button just pressed
            buttonPressed = true;
            buttonPressStart = currentTime;
            longPressHandled = false;
        } else if (!longPressHandled && (currentTime - buttonPressStart) >= BUTTON_LONG_PRESS_MS) {
            // Long press detected
            Serial.println("Long press detected - Running self-test");
            gimbalController.runSelfTest();
            longPressHandled = true;
        }
    } else {
        if (buttonPressed && !longPressHandled) {
            // Button released after short press
            if ((currentTime - buttonPressStart) >= BUTTON_DEBOUNCE_MS) {
                Serial.println("Short press detected - Setting flat reference");
                gimbalController.setFlatReference();
            }
        }
        buttonPressed = false;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== ESP32 3-Axis Gimbal System v1.2 ===");
    
    // Initialize button
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Run Power-On Self Test
    powerOnSelfTest();
    
    // Check critical failures
    if (!hwStatus.configOk) {
        Serial.println("CRITICAL: Config system failed! System halted.");
        while(true) { delay(1000); }
    }
    
    if (!hwStatus.sensorAvailable) {
        Serial.println("WARNING: Sensor not available. Auto mode will not work.");
        Serial.println("Continuing in degraded mode (manual control only).");
    }
    
    // Initialize WiFi
    wifiManager.begin();
    
    // Initialize Web Manager
    webManager.begin();
    
    // Initialize Bluetooth
    bluetoothManager.begin();
    
    // Connect Bluetooth Manager to Web Manager
    webManager.setBluetoothManager(&bluetoothManager);

    Serial.println("System Ready!");
}

void loop() {
    unsigned long currentTime = millis();
    static unsigned long lastSensorUpdate = 0;
    static unsigned long lastServoUpdate = 0;
    static unsigned long lastWSUpdate = 0;
    static unsigned long lastButtonCheck = 0;
    static unsigned long lastBTUpdate = 0;
    
    wifiManager.handle();
    webManager.handle();
    bluetoothManager.handle();
    
    // Handle button input
    if (currentTime - lastButtonCheck >= 10) { // Check button every 10ms
        handleButton();
        lastButtonCheck = currentTime;
    }
    
    // Update Sensors
    if (currentTime - lastSensorUpdate >= SENSOR_UPDATE_RATE) {
        if (hwStatus.sensorAvailable) {
            sensorManager.update();
        }
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
        float gyroYaw = 0, gyroPitch = 0, gyroRoll = 0;
        if (hwStatus.sensorAvailable) {
            gyroYaw = sensorManager.getGyroYaw() * dt * 57.2958;
            gyroPitch = sensorManager.getGyroPitch() * dt * 57.2958;
            gyroRoll = sensorManager.getGyroRoll() * dt * 57.2958;
        }

        gimbalController.update(dt, gyroYaw, gyroPitch, gyroRoll);
        lastServoUpdate = currentTime;
    }
    
    // WebSocket Broadcast
    if (currentTime - lastWSUpdate >= WEBSOCKET_UPDATE_RATE) {
        webManager.broadcastStatus();
        lastWSUpdate = currentTime;
    }
    
    // Bluetooth Status Update
    if (currentTime - lastBTUpdate >= WEBSOCKET_UPDATE_RATE) {
        bluetoothManager.updateStatus();
        lastBTUpdate = currentTime;
    }
}
