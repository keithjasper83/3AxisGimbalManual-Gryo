# ESP32 3-Axis Gimbal - Deployment Guide

## 🚀 Production Deployment Instructions

This guide covers deploying the ESP32 3-Axis Gimbal system to production hardware.

---

## Prerequisites

### Hardware Requirements
- **ESP32-S3-DevKitC-1** board (without PSRAM)
- **MPU6050** gyroscope/accelerometer sensor
- Three servo motors (compatible with PWM control)
- USB cable for programming
- Power supply (appropriate for servos)

### Software Requirements
- **PlatformIO CLI** or **PlatformIO IDE** (VS Code extension)
- USB drivers for ESP32 (CH340/CP2102)
- Python 3.7+ (for PlatformIO)

### Installation
```bash
# Install PlatformIO CLI
pip install platformio

# Or install VS Code extension:
# https://platformio.org/install/ide?install=vscode
```

---

## 📋 Step-by-Step Deployment

### Step 1: Clone the Repository
```bash
git clone https://github.com/keithjasper83/3AxisGimbalManual-Gryo.git
cd 3AxisGimbalManual-Gryo/esp32_firmware
```

### Step 2: Configure Hardware Pins (if needed)
Edit `include/config.h` to match your wiring:
```cpp
// Servo Pin Configuration
#define SERVO_PIN_YAW 13
#define SERVO_PIN_PITCH 12
#define SERVO_PIN_ROLL 14

// MPU6050 I2C Configuration
#define MPU6050_SDA 8
#define MPU6050_SCL 9

// RGB LED (ESP32-S3 onboard)
#define RGB_LED_PIN 48

// Button
#define BUTTON_PIN 15
```

### Step 3: Configure WiFi Credentials
⚠️ **SECURITY WARNING**: Change default passwords!

Edit `esp32_firmware/data/config.json`:
```json
{
  "wifi": {
    "ssid": "YourNetworkName",
    "password": "YourSecurePassword"
  },
  "hotspot": {
    "ssid": "Gimbal_AP",
    "password": "ChangeThisPassword123!"
  }
}
```

### Step 4: Build the Firmware
```bash
cd esp32_firmware
pio run
```

**Expected Output:**
```
Processing esp32dev (platform: espressif32; board: esp32-s3-devkitc-1; framework: arduino)
...
RAM:   [==        ]  XX.X% (used XXXXX bytes from XXXXX bytes)
Flash: [======    ]  XX.X% (used XXXXXX bytes from XXXXXX bytes)
========================= [SUCCESS] Took X.XX seconds =========================
```

### Step 5: Upload Firmware to ESP32
```bash
# Connect ESP32 via USB, then:
pio run --target upload

# Or specify port explicitly:
pio run --target upload --upload-port /dev/ttyUSB0  # Linux/Mac
pio run --target upload --upload-port COM3          # Windows
```

### Step 6: Upload Filesystem (Web GUI)
⚠️ **CRITICAL**: This step is required for the web interface to work!

```bash
# Upload LittleFS filesystem containing index.html and config.json
pio run --target uploadfs

# Or with specific port:
pio run --target uploadfs --upload-port /dev/ttyUSB0
```

**Files being uploaded:**
- `data/index.html` - Web interface (Dashboard, Config, Help)
- `data/config.json` - Runtime configuration
- `data/favicon.svg` - Website icon

### Step 7: Monitor Serial Output
```bash
pio device monitor

# Or with specific baud rate:
pio device monitor -b 115200
```

**Expected Boot Output:**
```
=== ESP32 3-Axis Gimbal System v1.2 ===

=== Power-On Self Test (POST) ===
Config System: OK
MPU6050 Sensor: OK
Servo Controllers: OK
=================================

Starting Hotspot...
AP IP: 192.168.4.1
Initializing Bluetooth...
Bluetooth BLE service started - Advertising as 'ESP32_Gimbal'
System Ready!
```

---

## 🔧 Troubleshooting

### Issue: "Failed to find MPU6050 chip"
**Symptoms:**
```
MPU6050 Sensor: Failed to find MPU6050 chip
FAILED (Manual mode only)
WARNING: Sensor not available. Auto mode will not work.
```

**Solutions:**
1. **Check Wiring**: Verify I2C connections (SDA=GPIO8, SCL=GPIO9)
2. **Check I2C Address**: MPU6050 uses 0x68 or 0x69
3. **Check Power**: MPU6050 needs 3.3V or 5V power
4. **Check Pull-ups**: I2C lines need 4.7kΩ pull-up resistors
5. **Test Continuity**: Use multimeter to verify connections

**Degraded Mode:**
The system will continue in manual mode only without the sensor.

### Issue: "PSRAM init failed"
**Symptoms:**
```
E (280) psram: PSRAM ID read error: 0x00ffffff
PSRAM init failed!
```

**Solution:**
✅ This warning is **FIXED** in the current version. The PSRAM flag has been removed from `platformio.ini`.

If you still see this:
1. Verify `platformio.ini` does NOT contain `-DBOARD_HAS_PSRAM`
2. Clean build: `pio run --target clean && pio run`

### Issue: "/littlefs/index.html does not exist"
**Symptoms:**
```
[E][vfs_api.cpp:105] open(): /littlefs/index.html does not exist
[E][vfs_api.cpp:105] open(): /littlefs/favicon.ico does not exist
```

**Solution:**
✅ **YOU MUST UPLOAD THE FILESYSTEM!**

```bash
pio run --target uploadfs
```

This uploads the web interface files to the ESP32's LittleFS partition.

### Issue: "Bluetooth not visible on iPhone"
**Symptoms:**
- ESP32 shows "Bluetooth BLE service started"
- iPhone Settings > Bluetooth doesn't show "ESP32_Gimbal"

**Solutions:**
1. **Use BLE Scanner App**: iOS hides BLE devices from Settings. Use a BLE scanner app:
   - **LightBlue** (recommended, free)
   - **nRF Connect** (Nordic Semiconductor)
   - **BLE Scanner** (Bluepixel Technologies)

2. **Check Advertising**: The device should appear as "ESP32_Gimbal"

3. **Distance**: Stay within 10 meters of the ESP32

4. **Restart**: Reboot both iPhone and ESP32

5. **iOS Permissions**: Ensure the app has Bluetooth permissions

**Note:** The current implementation uses BLE (Bluetooth Low Energy), not Classic Bluetooth. iOS Settings only shows Classic Bluetooth devices. You MUST use a BLE scanner app.

### Issue: Upload fails with "No serial port found"
**Solutions:**
1. **Install USB Drivers**:
   - CH340: https://sparks.gogo.co.nz/ch340.html
   - CP2102: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

2. **Check Connection**: Try a different USB cable (data cable, not charge-only)

3. **Press BOOT Button**: Hold BOOT button on ESP32 during upload

4. **Check Port**:
   ```bash
   # Linux
   ls /dev/ttyUSB* /dev/ttyACM*
   
   # macOS
   ls /dev/cu.* /dev/tty.*
   
   # Windows
   # Check Device Manager > Ports (COM & LPT)
   ```

---

## 🌐 Accessing the Web Interface

### Option 1: Access Point Mode (Default)
1. ESP32 creates WiFi hotspot: **"Gimbal_AP"**
2. Connect to it with password (from config.json)
3. Open browser: http://192.168.4.1

### Option 2: Station Mode (Connect to Your WiFi)
1. Configure WiFi in `data/config.json` before upload
2. ESP32 connects to your network
3. Check serial monitor for IP address
4. Open browser: http://[ESP32_IP_ADDRESS]

---

## 📱 Bluetooth Control (iPhone/Android)

### Connecting via BLE

1. **Download BLE Scanner App** (iPhone users MUST use an app):
   - **LightBlue** (iOS/Android) - Recommended
   - **nRF Connect** (iOS/Android)
   - **BLE Scanner** (Android)

2. **Scan for Devices**:
   - Open the app
   - Look for **"ESP32_Gimbal"**
   - Tap to connect

3. **BLE Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`

4. **Characteristics**:
   - **Position Control** (Write): `beb5483e-36e1-4688-b7f5-ea07361b26a8`
   - **Mode Control** (Write): `beb5483e-36e1-4688-b7f5-ea07361b26a9`
   - **Status** (Read/Notify): `beb5483e-36e1-4688-b7f5-ea07361b26aa`

### Custom App Development
For developing a custom iOS/Android app:

**iOS (Swift)**:
```swift
import CoreBluetooth

let serviceUUID = CBUUID(string: "4fafc201-1fb5-459e-8fcc-c5c9c331914b")
let positionUUID = CBUUID(string: "beb5483e-36e1-4688-b7f5-ea07361b26a8")
// ... connect and write data
```

**Android (Kotlin)**:
```kotlin
import android.bluetooth.*

val serviceUUID = UUID.fromString("4fafc201-1fb5-459e-8fcc-c5c9c331914b")
val positionUUID = UUID.fromString("beb5483e-36e1-4688-b7f5-ea07361b26a8")
// ... connect and write data
```

---

## 🔒 Security Hardening (REQUIRED for Production)

⚠️ **CRITICAL SECURITY ISSUES** - Address before deployment:

### 1. Change Default Passwords
Edit `data/config.json`:
```json
{
  "hotspot": {
    "password": "Use-Strong-Random-Password-Here-Min16Chars!"
  }
}
```

### 2. Enable WiFi WPA2/WPA3 Encryption
The hotspot uses WPA2 by default - ensure it's configured.

### 3. Add Bluetooth Pairing (Future Enhancement)
Current implementation has no BLE security. See `KnownIssues.MD` #ISSUE-010.

### 4. Implement HTTPS for Web Interface (Future Enhancement)
Current web server uses HTTP. Consider ESP32 HTTPS support for sensitive deployments.

---

## 📊 System Status LED

The onboard RGB LED indicates system status:

| Color | Meaning | Description |
|-------|---------|-------------|
| 🟢 **Green** | OK | All systems operational, sensor working |
| 🟡 **Yellow** | Warning | Degraded mode - sensor unavailable, manual only |
| 🔴 **Red** | Error | Critical failure - config system failed |

---

## 🔘 Physical Button Functions

**Button on GPIO 15:**
- **Short Press**: Set current position as "flat reference" (0,0,0)
- **Long Press (3s)**: Run servo self-test (sweep all axes)

---

## 🛠️ Development Workflow

### Build Commands
```bash
# Clean build
pio run --target clean
pio run

# Upload firmware only
pio run --target upload

# Upload filesystem only
pio run --target uploadfs

# Monitor serial output
pio device monitor

# All-in-one: build, upload firmware, upload filesystem, monitor
pio run --target upload && pio run --target uploadfs && pio device monitor
```

### Project Structure
```
esp32_firmware/
├── platformio.ini          # Build configuration
├── include/
│   └── config.h           # Hardware pin definitions, compile-time config
├── src/
│   ├── main.cpp           # Main application entry point
│   ├── Domain/            # Core business logic
│   │   ├── GimbalController.{h,cpp}
│   │   └── PIDController.{h,cpp}
│   ├── Services/          # Application services
│   │   ├── BluetoothManager.{h,cpp}
│   │   ├── ConfigManager.{h,cpp}
│   │   ├── LEDStatusManager.{h,cpp}
│   │   ├── WebManager.{h,cpp}
│   │   └── WiFiManager.{h,cpp}
│   └── Infrastructure/    # Hardware abstraction
│       └── SensorManager.{h,cpp}
└── data/                  # LittleFS filesystem
    ├── index.html         # Web interface (SPA)
    ├── config.json        # Runtime configuration
    └── favicon.svg        # Website icon
```

---

## 📚 Additional Documentation

- **README.md** - Project overview and quick start
- **KnownIssues.MD** - Known issues and security concerns
- **CONTRIBUTING.md** - Contribution guidelines
- **SECURITY.md** - Security policy and vulnerability reporting
- **docs/** - API documentation and additional guides

---

## ✅ Deployment Checklist

Before deploying to production:

- [ ] Changed default passwords in `data/config.json`
- [ ] Verified hardware wiring matches `include/config.h`
- [ ] Built firmware successfully (`pio run`)
- [ ] Uploaded firmware to ESP32 (`pio run --target upload`)
- [ ] **Uploaded filesystem** (`pio run --target uploadfs`) ⚠️ CRITICAL
- [ ] Verified web interface loads at http://192.168.4.1
- [ ] Tested servo movement (yaw, pitch, roll)
- [ ] Tested MPU6050 sensor (auto mode) or confirmed manual mode
- [ ] Verified Bluetooth visibility with BLE scanner app
- [ ] Tested physical button (short press, long press)
- [ ] Checked RGB LED status indicator
- [ ] Reviewed security warnings in `KnownIssues.MD`
- [ ] Documented any custom pin configurations
- [ ] Tested power supply stability under load

---

## 🆘 Support

- **Issues**: https://github.com/keithjasper83/3AxisGimbalManual-Gryo/issues
- **Discussions**: https://github.com/keithjasper83/3AxisGimbalManual-Gryo/discussions
- **Security**: See SECURITY.md for reporting vulnerabilities

---

## 📝 License

See LICENSE file for details.

---

**Last Updated**: 2026-01-30  
**Firmware Version**: v1.2  
**Compatible Hardware**: ESP32-S3-DevKitC-1 (no PSRAM required)
