#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"
#define HOTSPOT_SSID "Gimbal_AP"
#define HOTSPOT_PASSWORD "gimbal123"
#define WIFI_TIMEOUT 10000 // 10 seconds

// Servo Pin Configuration
#define SERVO_PIN_YAW 13
#define SERVO_PIN_PITCH 12
#define SERVO_PIN_ROLL 14

// MPU6050 Configuration (ESP32-S3 compatible pins)
#define MPU6050_SDA 8
#define MPU6050_SCL 9

// RGB LED Configuration (ESP32-S3-N16R8 onboard LED)
#define RGB_LED_PIN 48
#define RGB_LED_BRIGHTNESS 50 // 0-255, brightness level

// Button Configuration
#define BUTTON_PIN 15
#define BUTTON_DEBOUNCE_MS 50
#define BUTTON_LONG_PRESS_MS 3000

// Servo Limits (degrees)
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
#define SERVO_CENTER 90

// Auto Mode PID Parameters
#define KP 2.0
#define KI 0.5
#define KD 1.0

// WebServer Configuration
#define HTTP_PORT 80
#define WEBSOCKET_PORT 8080

// Update Rates (milliseconds)
#define SENSOR_UPDATE_RATE 10
#define SERVO_UPDATE_RATE 20
#define WEBSOCKET_UPDATE_RATE 100

// Operation Modes
#define MODE_MANUAL 0
#define MODE_AUTO 1

#endif
