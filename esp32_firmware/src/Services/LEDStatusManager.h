#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// LED Status States
enum class LEDStatus {
    OFF,              // LED off
    ERROR,            // RED - Critical error (config/filesystem failed)
    WARNING,          // YELLOW - Hardware missing (sensor not available)
    PARTIAL,          // GREEN (flashing) - Some hardware detected
    OK                // GREEN (solid) - All systems OK
};

class LEDStatusManager {
public:
    LEDStatusManager();
    void begin();
    void setStatus(LEDStatus status);
    void update(); // Call in loop to handle flashing
    
private:
    Adafruit_NeoPixel _pixel;
    LEDStatus _currentStatus;
    unsigned long _lastUpdate;
    bool _flashState;
    
    void _updateLED();
};
