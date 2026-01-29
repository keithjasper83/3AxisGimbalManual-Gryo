#pragma once
#include <WiFi.h>
#include "ConfigManager.h"

class WiFiManagerService {
public:
    WiFiManagerService(ConfigManager& configManager);
    void begin();
    void handle();
    bool isConnected();
    String getIP();

private:
    ConfigManager& _configManager;
    bool _isAPMode = false;
};
