# Optimal Wiring Layout for ESP32-S3-DevKitC-1

## Overview

This document provides the optimal pin configuration for the ESP32-S3-DevKitC-1 based gimbal system, designed to minimize wiring complexity by using **physically consecutive GPIO pins** for components with multiple connections.

## Key Benefits

✅ **Single header connections** - IMU and servos use consecutive pins  
✅ **Clean wiring** - No wire-to-wire connections needed  
✅ **Professional appearance** - Organized, minimal cable routing  
✅ **Easy assembly** - Use pre-made headers or ribbon cables  

## Pin Assignment Summary

### IMU (MPU6050) - 4-Pin Header
Uses **GPIO 10 & 11** which are physically adjacent on the ESP32-S3 board:

```
┌─────────────────────────────────┐
│ ESP32-S3 Left Side (Bottom)    │
├─────────────────────────────────┤
│ GPIO10 (SDA) ──┐                │
│ GPIO11 (SCL) ──┤ Consecutive!   │
│ GPIO12       ──┤                │
│ GPIO13       ──┤                │
│ GPIO14       ──┘                │
│ 5V                              │
│ GND                             │
└─────────────────────────────────┘
```

**Wiring:**
```
ESP32-S3 Pin    Wire Color    MPU6050 Module
──────────────────────────────────────────────
3.3V            Red           VCC
GND             Black         GND
GPIO10 (SDA)    Blue          SDA
GPIO11 (SCL)    Yellow        SCL
```

### Servos - 3-Pin Signal Header
Uses **GPIO 12, 13, 14** which are three consecutive pins:

```
ESP32-S3 Pin    Servo         Wire Color
──────────────────────────────────────────
GPIO12          Yaw           Orange/Yellow
GPIO13          Pitch         Orange/Yellow
GPIO14          Roll          Orange/Yellow

Note: Each servo also needs:
- 5V Power (Red wire) - from external 5V supply
- GND (Brown/Black wire) - common ground
```

### Complete Physical Layout

```
ESP32-S3-DevKitC-1 Left Side (Bottom Section)
═══════════════════════════════════════════════

    ┌───┐
    │ 8 │ GPIO8  (available)
    ├───┤
    │ 3 │ GPIO3  (RX - reserved)
    ├───┤
    │46 │ GPIO46 (available)
    ├───┤
    │ 9 │ GPIO9  (available)
    ├───┤
    │10 │ GPIO10 (SDA) ◄──┐
    ├───┤                 │ MPU6050
    │11 │ GPIO11 (SCL) ◄──┘ I2C Header
    ├───┤
    │12 │ GPIO12 (Yaw) ◄──┐
    ├───┤                 │
    │13 │ GPIO13 (Pitch) ◄┤ Servo
    ├───┤                 │ Signal
    │14 │ GPIO14 (Roll) ◄─┘ Header
    ├───┤
    │5V │ 5V
    ├───┤
    │GND│ GND
    └───┘
```

## Recommended Headers & Connectors

### For MPU6050 (4-Pin)
- **Option 1**: 4-pin female-to-female dupont jumper cable
- **Option 2**: 4-pin 0.1" (2.54mm) header with crimped connector
- **Pin Order**: 3.3V, GND, SDA(GPIO10), SCL(GPIO11)

### For Servos (Signal Only)
- **Option 1**: 3-pin female-to-female dupont jumper cables (one per servo)
- **Option 2**: 3-pin 0.1" (2.54mm) header strip with individual servo connectors
- **Pin Order**: GPIO12(Yaw), GPIO13(Pitch), GPIO14(Roll)

### For Servo Power
- Use a common power rail for all servos:
  - **5V rail**: Connect to external 5V power supply (3A+)
  - **GND rail**: Common ground with ESP32

## Wiring Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                        ESP32-S3                              │
│                                                              │
│  [3.3V]──────┐                                               │
│  [GND]───────┼──────┐                                        │
│  [GPIO10]────┼──────┼────┐                                   │
│  [GPIO11]────┼──────┼────┼────┐                              │
│  [GPIO12]────┼──────┼────┼────┼────┐                         │
│  [GPIO13]────┼──────┼────┼────┼────┼────┐                    │
│  [GPIO14]────┼──────┼────┼────┼────┼────┼────┐               │
│  [GPIO15]────┼──────┼────┼────┼────┼────┼────┼──┐ Button    │
└──────────────┼──────┼────┼────┼────┼────┼────┼──┼───────────┘
               │      │    │    │    │    │    │  │
               │      │    │    │    │    │    │  └─── [Button] ── GND
               │      │    │    │    │    │    │
        ┌──────▼──────▼────▼────▼─┐  │    │    │
        │    MPU6050 Module       │  │    │    │
        ├─────────────────────────┤  │    │    │
        │ VCC  GND  SDA  SCL      │  │    │    │
        └─────────────────────────┘  │    │    │
                                     │    │    │
        ┌────────────────────────────▼────▼────▼────┐
        │         Servo Signal Connections          │
        ├───────────────────────────────────────────┤
        │  GPIO12    GPIO13    GPIO14               │
        │   Yaw      Pitch     Roll                 │
        │    │         │         │                  │
        │    ▼         ▼         ▼                  │
        │  [Sig]     [Sig]     [Sig]                │
        │  [5V]*     [5V]*     [5V]*                │
        │  [GND]     [GND]     [GND]                │
        └───────────────────────────────────────────┘
                     │
                     │ *5V from external supply
                     │
            ┌────────▼─────────┐
            │  5V Power Supply │
            │     (3A+)        │
            └──────────────────┘
```

## Assembly Instructions

### Step 1: Prepare Headers
1. Cut or obtain:
   - One 4-pin female header/cable for MPU6050
   - Three individual servo extension cables OR one 3-pin header strip

### Step 2: Connect MPU6050
1. Orient the MPU6050 module with pins visible
2. Identify the pin order (usually printed): VCC, GND, SCL, SDA
3. Connect the 4-pin header:
   - ESP32 **3.3V** → MPU6050 **VCC**
   - ESP32 **GND** → MPU6050 **GND**
   - ESP32 **GPIO10** → MPU6050 **SDA**
   - ESP32 **GPIO11** → MPU6050 **SCL**

### Step 3: Connect Servos
1. Each servo has a 3-wire connector: Signal (Orange/Yellow), VCC (Red), GND (Brown/Black)
2. Connect servo signals:
   - **Yaw servo** signal → ESP32 **GPIO12**
   - **Pitch servo** signal → ESP32 **GPIO13**
   - **Roll servo** signal → ESP32 **GPIO14**
3. Connect all servo power wires (Red) to external **5V supply** positive
4. Connect all servo ground wires (Brown/Black) to common ground rail
5. Connect ESP32 **GND** to the common ground rail

### Step 4: Connect Button (Optional)
1. Connect one button terminal to ESP32 **GPIO15**
2. Connect other button terminal to ESP32 **GND**

### Step 5: Power
1. Connect external 5V supply (3A+) to servo power rail
2. Connect ESP32 via USB for programming and power
3. Ensure all grounds are connected together

## Pin Configuration Reference

| Component       | ESP32 Pin | Function    | Notes                          |
|-----------------|-----------|-------------|--------------------------------|
| **MPU6050**     |           |             |                                |
| - VCC           | 3.3V      | Power       | NOT 5V! Will damage sensor     |
| - GND           | GND       | Ground      |                                |
| - SDA           | GPIO10    | I2C Data    | Consecutive with SCL           |
| - SCL           | GPIO11    | I2C Clock   | Consecutive with SDA           |
| **Servos**      |           |             |                                |
| - Yaw Signal    | GPIO12    | PWM         | Consecutive pins               |
| - Pitch Signal  | GPIO13    | PWM         | Consecutive pins               |
| - Roll Signal   | GPIO14    | PWM         | Consecutive pins               |
| - Power (all)   | 5V Ext    | Power       | External supply, 3A+           |
| - Ground (all)  | GND       | Ground      | Common with ESP32              |
| **Button**      |           |             |                                |
| - Input         | GPIO15    | Digital In  | Pull-up enabled, active LOW    |
| **LED**         |           |             |                                |
| - Status        | GPIO48    | WS2812      | Internal RGB LED (S3-N16R8)    |

## Firmware Configuration

The pin assignments are defined in `esp32_firmware/include/config.h`:

```cpp
// Servo Pin Configuration (Consecutive pins for single header)
#define SERVO_PIN_YAW 12
#define SERVO_PIN_PITCH 13
#define SERVO_PIN_ROLL 14

// MPU6050 I2C Configuration (Consecutive pins for single header)
#define MPU6050_SDA 10
#define MPU6050_SCL 11

// Button
#define BUTTON_PIN 15

// RGB LED (Internal)
#define RGB_LED_PIN 48
```

## Troubleshooting

### Issue: MPU6050 not detected
- **Check**: Pins GPIO10 and GPIO11 are correctly connected to SDA and SCL
- **Check**: 3.3V power, not 5V (5V will damage the sensor!)
- **Try**: Swapping SDA/SCL if accidentally reversed
- **Try**: External 4.7kΩ pull-up resistors on SDA and SCL lines

### Issue: Servos not moving
- **Check**: Signal pins GPIO12, 13, 14 are connected correctly
- **Check**: External 5V supply is connected and providing sufficient current
- **Check**: Common ground between ESP32 and servo power supply
- **Check**: Servos themselves are functional (test with known good setup)

### Issue: Erratic servo movement
- **Check**: Power supply is adequate (3A+ for 3 servos)
- **Check**: Ground connections are solid
- **Try**: Add a large capacitor (1000µF) across servo power supply

## Additional Resources

- **Complete GPIO Reference**: See `docs/GPIO_PIN_REFERENCE.md`
- **Bench Testing Guide**: See `docs/BENCH_TEST_GUIDE.md`
- **Hardware Setup**: See `docs/HARDWARE_SETUP.md`
- **ESP32-S3 Datasheet**: [Espressif Documentation](https://www.espressif.com/en/products/socs/esp32-s3)

---

**Document Version**: 1.0  
**Last Updated**: February 2026  
**Hardware**: ESP32-S3-DevKitC-1 (ESP32-S3-N16R8)
