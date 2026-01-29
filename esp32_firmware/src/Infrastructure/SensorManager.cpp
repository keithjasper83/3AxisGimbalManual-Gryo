#include "SensorManager.h"

bool SensorManager::begin() {
    Wire.begin(MPU6050_SDA, MPU6050_SCL);
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        _sensorAvailable = false;
        return false;
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
    data.accelX = a.acceleration.x;
    data.accelY = a.acceleration.y;
    data.accelZ = a.acceleration.z;
    data.gyroX = g.gyro.x;
    data.gyroY = g.gyro.y;
    data.gyroZ = g.gyro.z;
    data.temp = temp.temperature;
    return data;
}

float SensorManager::getGyroYaw() {
    return g.gyro.z; // Keeping as rad/s for consistency with standard units, conversion handled in controller if needed
}

float SensorManager::getGyroPitch() {
    return g.gyro.y;
}

float SensorManager::getGyroRoll() {
    return g.gyro.x;
}
