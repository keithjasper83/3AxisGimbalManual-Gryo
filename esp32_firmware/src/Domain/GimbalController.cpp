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

    AppConfig config = _configManager.getConfig();
    updateServos(config);
}

void GimbalController::update(float dt, float gyroYaw, float gyroPitch, float gyroRoll) {
    // Get config before taking mutex to avoid lock-order inversion
    AppConfig config = _configManager.getConfig(); // Copy by value

    xSemaphoreTake(_mutex, portMAX_DELAY);

    // Update PID tunings
    _pidYaw.setTunings(config.kp, config.ki, config.kd);
    _pidPitch.setTunings(config.kp, config.ki, config.kd);
    _pidRoll.setTunings(config.kp, config.ki, config.kd);

    // Always update timed moves regardless of mode
    updateTimedMove();

    if (config.mode == MODE_AUTO) {
        updateAuto(dt, gyroYaw, gyroPitch, gyroRoll);
    }

    updateServos(config);

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

void GimbalController::updateServos(const AppConfig& config) {
    // Smoothing
    _currentPos.yaw += (_targetPos.yaw - _currentPos.yaw) * 0.1;
    _currentPos.pitch += (_targetPos.pitch - _currentPos.pitch) * 0.1;
    _currentPos.roll += (_targetPos.roll - _currentPos.roll) * 0.1;

    // Constrain logical position
    _currentPos.yaw = constrain(_currentPos.yaw, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    _currentPos.pitch = constrain(_currentPos.pitch, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    _currentPos.roll = constrain(_currentPos.roll, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

    // Apply configured servo offsets (trim) before writing to hardware, then clamp
    float yawCommand = constrain(_currentPos.yaw + config.yaw_offset, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    float pitchCommand = constrain(_currentPos.pitch + config.pitch_offset, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    float rollCommand = constrain(_currentPos.roll + config.roll_offset, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

    _servoYaw.write((int)yawCommand);
    _servoPitch.write((int)pitchCommand);
    _servoRoll.write((int)rollCommand);
}

void GimbalController::setMode(int mode) {
    // Get config and update it WITHOUT holding gimbal mutex to avoid lock-order inversion
    AppConfig config = _configManager.getConfig();
    config.mode = mode;
    _configManager.updateConfig(config);

    // Now take gimbal mutex to reset PIDs if needed
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
    // Center to the stored flat reference position instead of absolute center
    AppConfig config = _configManager.getConfig();
    float centerYaw = config.flat_ref_yaw > 0 ? config.flat_ref_yaw : SERVO_CENTER;
    float centerPitch = config.flat_ref_pitch > 0 ? config.flat_ref_pitch : SERVO_CENTER;
    float centerRoll = config.flat_ref_roll > 0 ? config.flat_ref_roll : SERVO_CENTER;
    setManualPosition(centerYaw, centerPitch, centerRoll);
}

void GimbalController::setFlatReference() {
    // Capture the current position as the new flat reference
    xSemaphoreTake(_mutex, portMAX_DELAY);
    GimbalPosition currentPos = _currentPos;
    xSemaphoreGive(_mutex);
    
    // Update config with new flat reference
    AppConfig config = _configManager.getConfig();
    config.flat_ref_yaw = currentPos.yaw;
    config.flat_ref_pitch = currentPos.pitch;
    config.flat_ref_roll = currentPos.roll;
    _configManager.updateConfig(config);
    
    Serial.println("Flat reference set to current position:");
    Serial.printf("  Yaw: %.2f, Pitch: %.2f, Roll: %.2f\n", 
                  currentPos.yaw, currentPos.pitch, currentPos.roll);
}

void GimbalController::runSelfTest() {
    Serial.println("=== Running Gimbal Self-Test ===");
    
    // Test 1: Servo range test
    Serial.println("Test 1: Servo Range Test");
    xSemaphoreTake(_mutex, portMAX_DELAY);
    
    // Save current position
    GimbalPosition originalPos = _currentPos;
    
    // Test each axis
    _targetPos = {SERVO_MIN_ANGLE, SERVO_CENTER, SERVO_CENTER};
    xSemaphoreGive(_mutex);
    delay(500);
    
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _targetPos = {SERVO_MAX_ANGLE, SERVO_CENTER, SERVO_CENTER};
    xSemaphoreGive(_mutex);
    delay(500);
    
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _targetPos = {SERVO_CENTER, SERVO_MIN_ANGLE, SERVO_CENTER};
    xSemaphoreGive(_mutex);
    delay(500);
    
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _targetPos = {SERVO_CENTER, SERVO_MAX_ANGLE, SERVO_CENTER};
    xSemaphoreGive(_mutex);
    delay(500);
    
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _targetPos = {SERVO_CENTER, SERVO_CENTER, SERVO_MIN_ANGLE};
    xSemaphoreGive(_mutex);
    delay(500);
    
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _targetPos = {SERVO_CENTER, SERVO_CENTER, SERVO_MAX_ANGLE};
    xSemaphoreGive(_mutex);
    delay(500);
    
    // Return to flat reference position if set, otherwise original position
    AppConfig config = _configManager.getConfig();
    xSemaphoreTake(_mutex, portMAX_DELAY);
    if (config.flat_ref_yaw > 0 || config.flat_ref_pitch > 0 || config.flat_ref_roll > 0) {
        _targetPos.yaw = config.flat_ref_yaw > 0 ? config.flat_ref_yaw : SERVO_CENTER;
        _targetPos.pitch = config.flat_ref_pitch > 0 ? config.flat_ref_pitch : SERVO_CENTER;
        _targetPos.roll = config.flat_ref_roll > 0 ? config.flat_ref_roll : SERVO_CENTER;
    } else {
        _targetPos = originalPos;
    }
    xSemaphoreGive(_mutex);
    
    Serial.println("Self-test complete!");
    Serial.println("================================");
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
