#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "config.h"

struct AppConfig {
    String wifi_ssid;
    String wifi_password;
    String hotspot_ssid;
    String hotspot_password;
    int mode;
    float kp;
    float ki;
    float kd;

    // Servo Trims/Offsets
    int yaw_offset;
    int pitch_offset;
    int roll_offset;
};

class ConfigManager {
public:
    ConfigManager();
    bool begin();
    bool loadConfig();
    bool saveConfig();
    AppConfig getConfig(); // Return by value
    void resetToDefaults();
    void updateConfig(const AppConfig& newConfig);

private:
    AppConfig config;
    const char* _filename = "/config.json";
    SemaphoreHandle_t _mutex;

    bool _saveConfigInternal(); // Internal helper without lock
};
