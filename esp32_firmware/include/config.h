#ifndef CONFIG_H
#define CONFIG_H

// ⚠️ SECURITY WARNING: See KnownIssues.MD #ISSUE-003, #ISSUE-004
// TODO: Change these default passwords before deployment!
// Default passwords are a CRITICAL security vulnerability.

// WiFi Configuration
#define ENFORCE_HOTSPOT true // Enforce hotspot mode for debugging/setup
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"  // ⚠️ CHANGE THIS!
#define HOTSPOT_SSID "Gimbal_AP"
#define HOTSPOT_PASSWORD "gimbal123"      // ⚠️ WEAK DEFAULT - CHANGE THIS!
#define WIFI_TIMEOUT 10000 // 10 seconds

// Servo Pin Configuration
// Using consecutive pins GPIO12, GPIO13, GPIO14 for single header connection
#define SERVO_PIN_YAW 12
#define SERVO_PIN_PITCH 13
#define SERVO_PIN_ROLL 14

// MPU6050 Configuration (ESP32-S3 compatible pins)
// Using consecutive pins GPIO10, GPIO11 for single header connection
// Pin order matches MPU6050 module: VCC(3V3), GND, SDA(GPIO10), SCL(GPIO11)
#define MPU6050_SDA 10
#define MPU6050_SCL 11

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

// Phone Gyro Rate Control
// Gyro input is rad/s from the phone; firmware converts to deg/s and applies gain.
#define PHONE_GYRO_GAIN_YAW 1.0f
#define PHONE_GYRO_GAIN_PITCH 1.0f
#define PHONE_GYRO_GAIN_ROLL 1.0f
#define PHONE_GYRO_DEADBAND_RAD_S 0.02f
#define PHONE_GYRO_TIMEOUT_MS 500

// Operation Modes
#define MODE_MANUAL 0
#define MODE_AUTO 1

#endif
