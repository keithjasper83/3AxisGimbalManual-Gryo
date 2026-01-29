#pragma once
#include <Arduino.h>

class PIDController {
public:
    PIDController(float kp, float ki, float kd);
    float compute(float setpoint, float input, float dt);
    void setTunings(float kp, float ki, float kd);
    void reset();

private:
    float _kp, _ki, _kd;
    float _integral;
    float _prevError;
};
