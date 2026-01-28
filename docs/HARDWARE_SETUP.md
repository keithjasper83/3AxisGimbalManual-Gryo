# Hardware Setup Guide

## Components Checklist

- [ ] ESP32 Development Board
- [ ] MPU6050 6-axis IMU
- [ ] 3x Servo Motors (SG90 or similar)
- [ ] 5V Power Supply (3A minimum)
- [ ] Jumper Wires
- [ ] Breadboard
- [ ] USB Cable (for programming)

## Step-by-Step Assembly

### 1. MPU6050 Connection

Connect the MPU6050 sensor to the ESP32:

| MPU6050 Pin | ESP32 Pin | Description |
|-------------|-----------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| SDA | GPIO 21 | I2C Data |
| SCL | GPIO 22 | I2C Clock |

**Important Notes:**
- MPU6050 operates at 3.3V - do NOT connect to 5V
- Keep I2C wires short (< 15cm) for reliable communication
- Add 4.7kΩ pull-up resistors if experiencing communication issues

### 2. Servo Motor Connections

Connect three servo motors for 3-axis control:

#### Yaw Servo (Base Rotation)
| Servo Wire | ESP32 Pin | Notes |
|------------|-----------|-------|
| Signal (Orange/Yellow) | GPIO 13 | PWM Control |
| Power (Red) | 5V | External power recommended |
| Ground (Brown/Black) | GND | Common ground with ESP32 |

#### Pitch Servo (Tilt)
| Servo Wire | ESP32 Pin | Notes |
|------------|-----------|-------|
| Signal (Orange/Yellow) | GPIO 12 | PWM Control |
| Power (Red) | 5V | External power recommended |
| Ground (Brown/Black) | GND | Common ground with ESP32 |

#### Roll Servo (Roll)
| Servo Wire | ESP32 Pin | Notes |
|------------|-----------|-------|
| Signal (Orange/Yellow) | GPIO 14 | PWM Control |
| Power (Red) | 5V | External power recommended |
| Ground (Brown/Black) | GND | Common ground with ESP32 |

**Power Considerations:**
- Servos under load can draw 500mA-1A each
- Use external 5V power supply for servos (not USB)
- Connect ESP32 GND to power supply GND (common ground)
- Consider using a capacitor (1000µF) near servos for stability

### 3. Power Supply Setup

#### Option A: Separate Power Supplies
```
┌──────────┐
│ 5V 3A PSU├──→ Servos VCC
└────┬─────┘
     └──→ ESP32 5V (via VIN pin)
     
GND common to all components
```

#### Option B: Buck Converter
```
┌──────────┐     ┌────────────┐
│ 12V PSU  ├──→──┤Buck Conv.  ├──→ 5V to Servos & ESP32
└──────────┘     │ (12V→5V)   │
                 └────────────┘
```

### 4. Wiring Diagram

```
                    ┌─────────────┐
                    │   ESP32     │
                    │             │
    MPU6050         │ GPIO21 (SDA)├──┐
    ┌─────┐         │ GPIO22 (SCL)├──┼──┐
    │ VCC ├─────────┤ 3.3V        │  │  │
    │ GND ├─────────┤ GND         │  │  │
    │ SDA ├─────────┤             │  │  │
    │ SCL ├─────────┤             │  │  │
    └─────┘         │ GPIO13      ├──┼──┼──→ Yaw Servo
                    │ GPIO12      ├──┼──┼──→ Pitch Servo
                    │ GPIO14      ├──┘  └──→ Roll Servo
                    │ VIN (5V)    ├────────→ 5V Power
                    │ GND         ├────────→ GND
                    └─────────────┘
```

### 5. Mechanical Assembly

#### Basic Gimbal Structure
1. Mount Yaw servo as base (fixed to platform)
2. Mount Pitch servo on Yaw servo arm
3. Mount Roll servo on Pitch servo arm
4. Attach payload (camera, sensor, etc.) to Roll servo

#### Tips:
- Balance the gimbal mechanically before powering
- Use proper mounting brackets for stability
- Consider using ball bearings for smoother operation
- Keep center of gravity aligned with rotation axes

## Testing

### 1. Power Test
```bash
# Upload firmware
cd esp32_firmware
pio run --target upload

# Open serial monitor
pio device monitor
```

Expected output:
```
=== ESP32 3-Axis Gimbal System ===
Initializing MPU6050...
MPU6050 initialized!
Initializing servos...
Servos initialized!
Connecting to WiFi...
WiFi connected!
IP address: 192.168.1.100
Web server started!
System ready!
```

### 2. MPU6050 Test
In serial monitor, you should see sensor updates without errors.
If "Failed to find MPU6050" appears:
- Check I2C wiring
- Verify 3.3V power to MPU6050
- Try I2C scanner sketch

### 3. Servo Test
1. Access web interface at ESP32's IP address
2. Move each slider individually
3. Verify corresponding servo moves smoothly
4. Check for jitter or erratic movement

### 4. Full System Test
1. Set mode to "Manual"
2. Move all three axes simultaneously
3. Switch to "Auto" mode
4. Verify gimbal stabilizes when tilted
5. Test timed moves

## Troubleshooting

### Issue: MPU6050 Not Detected
**Solutions:**
- Check wiring, especially SDA/SCL
- Verify 3.3V power (not 5V!)
- Check I2C address (try 0x68 or 0x69)
- Add pull-up resistors (4.7kΩ)

### Issue: Servos Jittering
**Solutions:**
- Add capacitor (1000µF) near servos
- Use external power supply
- Check for loose connections
- Reduce servo update rate in config

### Issue: System Resets/Brown-outs
**Solutions:**
- Use adequate power supply (3A minimum)
- Add bulk capacitors (1000-2200µF)
- Separate ESP32 and servo power
- Check voltage under load

### Issue: Limited Range of Motion
**Solutions:**
- Adjust servo limits in config.h
- Calibrate servo endpoints
- Check mechanical constraints
- Verify PWM pulse width settings

## Advanced Modifications

### Using Different Servos
Edit `config.h`:
```cpp
#define SERVO_MIN_PULSE 500   // Adjust for your servos
#define SERVO_MAX_PULSE 2500  // Adjust for your servos
```

### Adding More Sensors
- IMU upgrades (BNO055, MPU9250)
- GPS module for outdoor tracking
- Camera for FPV
- Distance sensors for obstacle detection

### Using Different Pins
Modify pin assignments in `config.h`:
```cpp
#define SERVO_PIN_YAW 13      // Change as needed
#define SERVO_PIN_PITCH 12    // Change as needed
#define SERVO_PIN_ROLL 14     // Change as needed
#define MPU6050_SDA 21        // Change as needed
#define MPU6050_SCL 22        // Change as needed
```

## Safety Notes

⚠️ **Important Safety Information:**
- Never connect servos directly to USB power
- Always use common ground between all components
- Secure all connections before applying power
- Test with low-weight payload first
- Monitor temperature of servos during operation
- Use proper fusing on power supply

## Next Steps

After successful hardware setup:
1. [Configure firmware settings](../esp32_firmware/include/config.h)
2. [Calibrate PID parameters](CALIBRATION.md)
3. [Set up web interface](../README.md#usage)
4. [Create preset moves](../README.md#api-usage)
