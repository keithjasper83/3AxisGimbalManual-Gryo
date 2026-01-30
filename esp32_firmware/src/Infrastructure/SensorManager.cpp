#include "SensorManager.h"

bool SensorManager::begin() {
    // Initialize I2C bus only once with custom pins
    Wire.begin(MPU6050_SDA, MPU6050_SCL);
    
    // Try standard I2C addresses: 0x68 (default) then 0x69 (alternate)
    if (!mpu.begin(0x68, &Wire, 0)) {
        // Try alternate address
        if (!mpu.begin(0x69, &Wire, 0)) {
            Serial.println("Failed to find MPU6050 chip");
            _sensorAvailable = false;
            return false;
        }
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    _sensorAvailable = true;
    return true;
}

void SensorManager::update() {
    if (_sensorAvailable) {
        mpu.getEvent(&a, &g, &temp);
    }
}

SensorData SensorManager::getData() {
    SensorData data;
    if (_sensorAvailable) {
        data.accelX = a.acceleration.x;
        data.accelY = a.acceleration.y;
        data.accelZ = a.acceleration.z;
        data.gyroX = g.gyro.x;
        data.gyroY = g.gyro.y;
        data.gyroZ = g.gyro.z;
        data.temp = temp.temperature;
    } else {
        // Return zeros when sensor is not available
        data.accelX = 0.0;
        data.accelY = 0.0;
        data.accelZ = 0.0;
        data.gyroX = 0.0;
        data.gyroY = 0.0;
        data.gyroZ = 0.0;
        data.temp = 0.0;
    }
    return data;
}

float SensorManager::getGyroYaw() {
    return _sensorAvailable ? g.gyro.z : 0.0;
}

float SensorManager::getGyroPitch() {
    return _sensorAvailable ? g.gyro.y : 0.0;
}

float SensorManager::getGyroRoll() {
    return _sensorAvailable ? g.gyro.x : 0.0;
}
