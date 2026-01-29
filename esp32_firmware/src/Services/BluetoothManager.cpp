#include "BluetoothManager.h"

BluetoothManager::BluetoothManager(GimbalController& gimbalController)
    : _gimbalController(gimbalController),
      _pServer(nullptr),
      _pPositionCharacteristic(nullptr),
      _pModeCharacteristic(nullptr),
      _pStatusCharacteristic(nullptr),
      _deviceConnected(false),
      _oldDeviceConnected(false)
{}

void BluetoothManager::ServerCallbacks::onConnect(BLEServer* pServer) {
    _manager->_deviceConnected = true;
    Serial.println("BLE Client Connected");
}

void BluetoothManager::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    _manager->_deviceConnected = false;
    Serial.println("BLE Client Disconnected");
}

void BluetoothManager::PositionCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    if (value.length() == 12) { // 3 floats x 4 bytes each
        float yaw, pitch, roll;
        memcpy(&yaw, &value[0], 4);
        memcpy(&pitch, &value[4], 4);
        memcpy(&roll, &value[8], 4);
        
        // Validate ranges (0-180 degrees)
        if (yaw < 0.0f || yaw > 180.0f || 
            pitch < 0.0f || pitch > 180.0f || 
            roll < 0.0f || roll > 180.0f) {
            Serial.println("BLE Position: Invalid values, ignoring");
            return;
        }
        
        Serial.printf("BLE Position: Yaw=%.1f, Pitch=%.1f, Roll=%.1f\n", yaw, pitch, roll);
        _manager->_gimbalController.setManualPosition(yaw, pitch, roll);
    }
}

void BluetoothManager::ModeCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    if (value.length() == 1) {
        int mode = value[0];
        // Validate mode (0 = Manual, 1 = Auto)
        if (mode != 0 && mode != 1) {
            Serial.printf("BLE Mode Change: Invalid mode %d, ignoring\n", mode);
            return;
        }
        Serial.printf("BLE Mode Change: %d\n", mode);
        _manager->_gimbalController.setMode(mode);
    }
}

void BluetoothManager::begin() {
    Serial.println("Initializing Bluetooth...");
    
    // Create the BLE Device
    BLEDevice::init("ESP32_Gimbal");
    
    // Create the BLE Server
    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(new ServerCallbacks(this));
    
    // Create the BLE Service
    BLEService *pService = _pServer->createService(SERVICE_UUID);
    
    // Create Position Characteristic (Write)
    _pPositionCharacteristic = pService->createCharacteristic(
        POSITION_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    _pPositionCharacteristic->setCallbacks(new PositionCallbacks(this));
    
    // Create Mode Characteristic (Write)
    _pModeCharacteristic = pService->createCharacteristic(
        MODE_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    _pModeCharacteristic->setCallbacks(new ModeCallbacks(this));
    
    // Create Status Characteristic (Read/Notify)
    _pStatusCharacteristic = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _pStatusCharacteristic->addDescriptor(new BLE2902());
    
    // Start the service
    pService->start();
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);  // Fixed: was setMinPreferred
    BLEDevice::startAdvertising();
    
    Serial.println("Bluetooth BLE service started - Advertising as 'ESP32_Gimbal'");
}

void BluetoothManager::handle() {
    // Handle connection state changes
    if (!_deviceConnected && _oldDeviceConnected) {
        // Device disconnected - restart advertising after brief pause
        static unsigned long lastDisconnect = 0;
        if (millis() - lastDisconnect > 500) {
            _pServer->startAdvertising();
            Serial.println("Start advertising");
            _oldDeviceConnected = _deviceConnected;
            lastDisconnect = millis();
        }
    }
    
    if (_deviceConnected && !_oldDeviceConnected) {
        _oldDeviceConnected = _deviceConnected;
    }
}

bool BluetoothManager::isConnected() {
    return _deviceConnected;
}

void BluetoothManager::updateStatus() {
    if (_deviceConnected && _pStatusCharacteristic) {
        GimbalPosition pos = _gimbalController.getCurrentPosition();
        int mode = _gimbalController.getMode();
        
        // Pack status into bytes: mode (1 byte) + yaw (4 bytes) + pitch (4 bytes) + roll (4 bytes)
        uint8_t status[13];
        status[0] = mode;
        memcpy(&status[1], &pos.yaw, 4);
        memcpy(&status[5], &pos.pitch, 4);
        memcpy(&status[9], &pos.roll, 4);
        
        _pStatusCharacteristic->setValue(status, 13);
        _pStatusCharacteristic->notify();
    }
}
