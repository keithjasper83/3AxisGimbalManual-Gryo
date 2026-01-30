#include "LEDStatusManager.h"

#define RGB_LED_PIN 48       // ESP32-S3-N16R8 onboard RGB LED
#define NUM_PIXELS 1
#define FLASH_INTERVAL 500   // Flash interval in ms

LEDStatusManager::LEDStatusManager() 
    : _pixel(NUM_PIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800),
      _currentStatus(LEDStatus::OFF),
      _lastUpdate(0),
      _flashState(false) {
}

void LEDStatusManager::begin() {
    _pixel.begin();
    _pixel.setBrightness(50); // Set to 50% brightness to avoid being too bright
    _pixel.show(); // Initialize all pixels to 'off'
}

void LEDStatusManager::setStatus(LEDStatus status) {
    _currentStatus = status;
    _updateLED();
}

void LEDStatusManager::update() {
    // Only update for flashing states
    if (_currentStatus == LEDStatus::PARTIAL) {
        unsigned long currentTime = millis();
        if (currentTime - _lastUpdate >= FLASH_INTERVAL) {
            _flashState = !_flashState;
            _lastUpdate = currentTime;
            _updateLED();
        }
    }
}

void LEDStatusManager::_updateLED() {
    switch (_currentStatus) {
        case LEDStatus::OFF:
            _pixel.setPixelColor(0, _pixel.Color(0, 0, 0)); // Off
            break;
            
        case LEDStatus::ERROR:
            _pixel.setPixelColor(0, _pixel.Color(255, 0, 0)); // Red
            break;
            
        case LEDStatus::WARNING:
            _pixel.setPixelColor(0, _pixel.Color(255, 255, 0)); // Yellow
            break;
            
        case LEDStatus::PARTIAL:
            if (_flashState) {
                _pixel.setPixelColor(0, _pixel.Color(0, 255, 0)); // Green
            } else {
                _pixel.setPixelColor(0, _pixel.Color(0, 0, 0)); // Off
            }
            break;
            
        case LEDStatus::OK:
            _pixel.setPixelColor(0, _pixel.Color(0, 255, 0)); // Green (solid)
            break;
    }
    _pixel.show();
}
