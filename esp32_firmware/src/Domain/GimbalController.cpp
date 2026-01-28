#include "GimbalController.h"

GimbalController::GimbalController(ConfigManager& configManager)
    : _configManager(configManager),
      _pidYaw(configManager.getConfig().kp, configManager.getConfig().ki, configManager.getConfig().kd),
      _pidPitch(configManager.getConfig().kp, configManager.getConfig().ki, configManager.getConfig().kd),
      _pidRoll(configManager.getConfig().kp, configManager.getConfig().ki, configManager.getConfig().kd)
{
    _currentPos = {90, 90, 90};
    _targetPos = {90, 90, 90};
    _autoTarget = {90, 90, 90};
    _moveActive = false;
    _mutex = xSemaphoreCreateMutex();
}

void GimbalController::begin() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);

    _servoYaw.setPeriodHertz(50);
    _servoPitch.setPeriodHertz(50);
    _servoRoll.setPeriodHertz(50);

    _servoYaw.attach(SERVO_PIN_YAW, 500, 2500);
    _servoPitch.attach(SERVO_PIN_PITCH, 500, 2500);
    _servoRoll.attach(SERVO_PIN_ROLL, 500, 2500);

    updateServos();
}

void GimbalController::update(float dt, float gyroYaw, float gyroPitch, float gyroRoll) {
    xSemaphoreTake(_mutex, portMAX_DELAY);

    AppConfig config = _configManager.getConfig(); // Copy by value

    // Update PID tunings
    _pidYaw.setTunings(config.kp, config.ki, config.kd);
    _pidPitch.setTunings(config.kp, config.ki, config.kd);
    _pidRoll.setTunings(config.kp, config.ki, config.kd);

    if (config.mode == MODE_AUTO) {
        updateAuto(dt, gyroYaw, gyroPitch, gyroRoll);
    } else {
        updateTimedMove();
    }

    updateServos();

    xSemaphoreGive(_mutex);
}

void GimbalController::updateAuto(float dt, float gyroYaw, float gyroPitch, float gyroRoll) {
    float errorYaw = _autoTarget.yaw - (_currentPos.yaw + gyroYaw);
    float errorPitch = _autoTarget.pitch - (_currentPos.pitch + gyroPitch);
    float errorRoll = _autoTarget.roll - (_currentPos.roll + gyroRoll);

    float correctionYaw = _pidYaw.compute(0, -errorYaw, dt);
    float correctionPitch = _pidPitch.compute(0, -errorPitch, dt);
    float correctionRoll = _pidRoll.compute(0, -errorRoll, dt);

    _targetPos.yaw = _currentPos.yaw + correctionYaw;
    _targetPos.pitch = _currentPos.pitch + correctionPitch;
    _targetPos.roll = _currentPos.roll + correctionRoll;
}

void GimbalController::updateTimedMove() {
    if (!_moveActive) return;

    unsigned long elapsed = millis() - _moveStartTime;

    if (elapsed >= _moveDuration) {
        _moveActive = false;
        _targetPos = _moveEndPos;
        return;
    }

    float progress = (float)elapsed / _moveDuration;
    _targetPos.yaw = _moveStartPos.yaw + (_moveEndPos.yaw - _moveStartPos.yaw) * progress;
    _targetPos.pitch = _moveStartPos.pitch + (_moveEndPos.pitch - _moveStartPos.pitch) * progress;
    _targetPos.roll = _moveStartPos.roll + (_moveEndPos.roll - _moveStartPos.roll) * progress;
}

void GimbalController::updateServos() {
    // Smoothing
    _currentPos.yaw += (_targetPos.yaw - _currentPos.yaw) * 0.1;
    _currentPos.pitch += (_targetPos.pitch - _currentPos.pitch) * 0.1;
    _currentPos.roll += (_targetPos.roll - _currentPos.roll) * 0.1;

    // Constrain
    _currentPos.yaw = constrain(_currentPos.yaw, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    _currentPos.pitch = constrain(_currentPos.pitch, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    _currentPos.roll = constrain(_currentPos.roll, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

    _servoYaw.write((int)_currentPos.yaw);
    _servoPitch.write((int)_currentPos.pitch);
    _servoRoll.write((int)_currentPos.roll);
}

void GimbalController::setMode(int mode) {
    // Mode is stored in config, but we might need to reset PIDs
    // ConfigManager has its own mutex, so we don't need to lock to set config
    AppConfig config = _configManager.getConfig();
    config.mode = mode;
    _configManager.updateConfig(config);

    xSemaphoreTake(_mutex, portMAX_DELAY);
    if (mode == MODE_MANUAL) {
        _pidYaw.reset();
        _pidPitch.reset();
        _pidRoll.reset();
    }
    xSemaphoreGive(_mutex);
}

int GimbalController::getMode() {
    return _configManager.getConfig().mode;
}

void GimbalController::setManualPosition(float yaw, float pitch, float roll) {
    if (_configManager.getConfig().mode == MODE_MANUAL) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        _targetPos = {yaw, pitch, roll};
        _moveActive = false; // Cancel any timed move
        xSemaphoreGive(_mutex);
    }
}

void GimbalController::setAutoTarget(float yaw, float pitch, float roll) {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _autoTarget = {yaw, pitch, roll};
    xSemaphoreGive(_mutex);
}

GimbalPosition GimbalController::getCurrentPosition() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    GimbalPosition pos = _currentPos;
    xSemaphoreGive(_mutex);
    return pos;
}

void GimbalController::center() {
    setManualPosition(SERVO_CENTER, SERVO_CENTER, SERVO_CENTER);
}

void GimbalController::startTimedMove(float duration, GimbalPosition endPos) {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _moveActive = true;
    _moveStartTime = millis();
    _moveDuration = duration;
    _moveStartPos = _currentPos;
    _moveEndPos = endPos;
    xSemaphoreGive(_mutex);
}
