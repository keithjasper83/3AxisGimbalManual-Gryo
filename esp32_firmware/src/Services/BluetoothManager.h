#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "../Domain/GimbalController.h"

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define POSITION_CHAR_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define MODE_CHAR_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define STATUS_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define BLE_DEVICE_NAME     "ESP32_Gimbal"

class BluetoothManager {
public:
    BluetoothManager(GimbalController& gimbalController);
    void begin();
    void handle();
    bool isConnected();
    void updateStatus();

private:
    GimbalController& _gimbalController;
    BLEServer* _pServer;
    BLECharacteristic* _pPositionCharacteristic;
    BLECharacteristic* _pModeCharacteristic;
    BLECharacteristic* _pStatusCharacteristic;
    volatile bool _deviceConnected;  // Accessed from BLE callback task
    volatile bool _oldDeviceConnected;

    class ServerCallbacks : public BLEServerCallbacks {
        BluetoothManager* _manager;
    public:
        ServerCallbacks(BluetoothManager* manager) : _manager(manager) {}
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer);
    };

    class PositionCallbacks : public BLECharacteristicCallbacks {
        BluetoothManager* _manager;
    public:
        PositionCallbacks(BluetoothManager* manager) : _manager(manager) {}
        void onWrite(BLECharacteristic* pCharacteristic);
    };

    class ModeCallbacks : public BLECharacteristicCallbacks {
        BluetoothManager* _manager;
    public:
        ModeCallbacks(BluetoothManager* manager) : _manager(manager) {}
        void onWrite(BLECharacteristic* pCharacteristic);
    };
};
