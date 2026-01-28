#include "PIDController.h"

PIDController::PIDController(float kp, float ki, float kd) : _kp(kp), _ki(ki), _kd(kd), _integral(0), _prevError(0) {}

float PIDController::compute(float setpoint, float input, float dt) {
    if (dt <= 0) return 0;

    float error = setpoint - input;
    _integral += error * dt;

    // Anti-windup clamping could be added here

    float derivative = (error - _prevError) / dt;
    _prevError = error;

    return (_kp * error) + (_ki * _integral) + (_kd * derivative);
}

void PIDController::setTunings(float kp, float ki, float kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

void PIDController::reset() {
    _integral = 0;
    _prevError = 0;
}
