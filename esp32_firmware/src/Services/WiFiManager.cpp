#include "WiFiManager.h"

WiFiManagerService::WiFiManagerService(ConfigManager& configManager) : _configManager(configManager) {}

void WiFiManagerService::begin() {
    AppConfig config = _configManager.getConfig();

    if (config.wifi_ssid.length() > 0 && config.wifi_ssid != "YourWiFiSSID") {
        Serial.printf("Connecting to %s...\n", config.wifi_ssid.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            delay(100);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi Connected!");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            _isAPMode = false;
            return;
        }
    }

    Serial.println("\nStarting Hotspot...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.hotspot_ssid.c_str(), config.hotspot_password.c_str());
    _isAPMode = true;
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void WiFiManagerService::handle() {
    // Reconnection logic could go here
    if (!_isAPMode && WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Lost, reconnecting...");
        WiFi.reconnect();
    }
}

bool WiFiManagerService::isConnected() {
    return WiFi.status() == WL_CONNECTED || _isAPMode;
}

String WiFiManagerService::getIP() {
    if (_isAPMode) return WiFi.softAPIP().toString();
    return WiFi.localIP().toString();
}
