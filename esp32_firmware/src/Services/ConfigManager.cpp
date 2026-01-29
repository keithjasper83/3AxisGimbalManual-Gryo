#include "ConfigManager.h"

ConfigManager::ConfigManager() {
    _mutex = xSemaphoreCreateMutex();
    resetToDefaults();
}

void ConfigManager::resetToDefaults() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    config.wifi_ssid = WIFI_SSID;
    config.wifi_password = WIFI_PASSWORD;
    config.hotspot_ssid = HOTSPOT_SSID;
    config.hotspot_password = HOTSPOT_PASSWORD;
    config.mode = MODE_MANUAL;
    config.kp = KP;
    config.ki = KI;
    config.kd = KD;
    config.yaw_offset = 0;
    config.pitch_offset = 0;
    config.roll_offset = 0;
    xSemaphoreGive(_mutex);
}

bool ConfigManager::begin() {
    if (!LittleFS.begin(false)) {
        Serial.println("LittleFS Mount Failed");
        return false;
    }
    return loadConfig();
}

bool ConfigManager::loadConfig() {
    xSemaphoreTake(_mutex, portMAX_DELAY);

    if (!LittleFS.exists(_filename)) {
        Serial.println("Config file not found, creating default.");
        xSemaphoreGive(_mutex);
        saveConfig(); // This will lock again
        return true;
    }

    File file = LittleFS.open(_filename, "r");
    if (!file) {
        Serial.println("Failed to open config file");
        xSemaphoreGive(_mutex);
        return false;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Failed to read file, using defaults and persisting them");
        // Release the mutex before calling methods that will lock it again.
        xSemaphoreGive(_mutex);
        // Ensure we actually revert to defaults and persist them to fix the file.
        resetToDefaults();
        bool saved = saveConfig();
        return saved;
    }

    if (doc.containsKey("wifi_ssid")) config.wifi_ssid = doc["wifi_ssid"].as<String>();
    if (doc.containsKey("wifi_password")) config.wifi_password = doc["wifi_password"].as<String>();
    if (doc.containsKey("hotspot_ssid")) config.hotspot_ssid = doc["hotspot_ssid"].as<String>();
    if (doc.containsKey("hotspot_password")) config.hotspot_password = doc["hotspot_password"].as<String>();

    config.mode = doc["mode"] | config.mode;
    config.kp = doc["kp"] | config.kp;
    config.ki = doc["ki"] | config.ki;
    config.kd = doc["kd"] | config.kd;

    config.yaw_offset = doc["yaw_offset"] | config.yaw_offset;
    config.pitch_offset = doc["pitch_offset"] | config.pitch_offset;
    config.roll_offset = doc["roll_offset"] | config.roll_offset;

    xSemaphoreGive(_mutex);
    return true;
}

bool ConfigManager::saveConfig() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    bool result = _saveConfigInternal();
    xSemaphoreGive(_mutex);
    return result;
}

bool ConfigManager::_saveConfigInternal() {
    StaticJsonDocument<1024> doc;
    doc["wifi_ssid"] = config.wifi_ssid;
    doc["wifi_password"] = config.wifi_password;
    doc["hotspot_ssid"] = config.hotspot_ssid;
    doc["hotspot_password"] = config.hotspot_password;
    doc["mode"] = config.mode;
    doc["kp"] = config.kp;
    doc["ki"] = config.ki;
    doc["kd"] = config.kd;
    doc["yaw_offset"] = config.yaw_offset;
    doc["pitch_offset"] = config.pitch_offset;
    doc["roll_offset"] = config.roll_offset;

    File file = LittleFS.open(_filename, "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    serializeJson(doc, file);
    file.close();
    return true;
}

AppConfig ConfigManager::getConfig() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    AppConfig c = config;
    xSemaphoreGive(_mutex);
    return c;
}

void ConfigManager::updateConfig(const AppConfig& newConfig) {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    config = newConfig;
    _saveConfigInternal();
    xSemaphoreGive(_mutex);
}
