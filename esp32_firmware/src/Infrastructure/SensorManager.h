#pragma once
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "config.h"

struct SensorData {
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    float temp;
};

class SensorManager {
public:
    bool begin();
    void update();
    SensorData getData();

    float getGyroYaw();
    float getGyroPitch();
    float getGyroRoll();
    
    bool isAvailable() const { return _sensorAvailable; }

private:
    Adafruit_MPU6050 mpu;
    sensors_event_t a, g, temp;
    bool _sensorAvailable = false;
};
