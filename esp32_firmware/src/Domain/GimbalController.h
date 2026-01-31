#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "PIDController.h"
#include "../Services/ConfigManager.h"

struct GimbalPosition {
    float yaw;
    float pitch;
    float roll;
};

class GimbalController {
public:
    GimbalController(ConfigManager& configManager);
    void begin();
    void update(float dt, float gyroYaw, float gyroPitch, float gyroRoll);

    void setMode(int mode);
    int getMode();

    void setManualPosition(float yaw, float pitch, float roll);
    void setAutoTarget(float yaw, float pitch, float roll);
    void setPhoneGyroRates(float gx, float gy, float gz);
    void clearPhoneGyro();

    GimbalPosition getCurrentPosition();
    void center();
    
    void setFlatReference(); // Set current position as new flat reference
    void runSelfTest(); // Run self-test routine

    void startTimedMove(float duration, GimbalPosition endPos);

private:
    ConfigManager& _configManager;
    Servo _servoYaw, _servoPitch, _servoRoll;
    PIDController _pidYaw, _pidPitch, _pidRoll;

    GimbalPosition _currentPos;
    GimbalPosition _targetPos;
    GimbalPosition _autoTarget;
    GimbalPosition _phoneGyroRates;
    uint32_t _phoneGyroLastMs;
    bool _phoneGyroActive;

    // Timed Move State
    bool _moveActive;
    unsigned long _moveStartTime;
    float _moveDuration;
    GimbalPosition _moveStartPos;
    GimbalPosition _moveEndPos;

    SemaphoreHandle_t _mutex;

    void updateServos(const AppConfig& config);
    void updateAuto(float dt, float gyroYaw, float gyroPitch, float gyroRoll);
    void updatePhoneGyro(float dt);
    void updateTimedMove();
};
