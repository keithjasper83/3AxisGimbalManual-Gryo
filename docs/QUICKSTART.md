# Quick Start Guide

Get your ESP32 3-Axis Gimbal running in 15 minutes!

## What You'll Need

### Hardware
- ESP32 development board
- MPU6050 sensor module
- 3x hobby servo motors (e.g., SG90)
- 5V power supply (3A recommended)
- Jumper wires
- USB cable for programming

### Software
- PlatformIO (installation instructions below)
- A computer with Python 3.8+ (optional, for backend)
- Web browser (Chrome, Firefox, Safari, Edge)

## Step 1: Install PlatformIO (5 minutes)

Choose one method:

### Method A: VS Code Extension (Recommended)
1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install PlatformIO IDE extension
3. Restart VS Code

### Method B: Command Line
```bash
pip install platformio
```

## Step 2: Hardware Assembly (5 minutes)

### Quick Wiring

**MPU6050 to ESP32:**
```
MPU6050 VCC → ESP32 3.3V
MPU6050 GND → ESP32 GND
MPU6050 SDA → ESP32 GPIO21
MPU6050 SCL → ESP32 GPIO22
```

**Servos to ESP32:**
```
Yaw Servo Signal   → ESP32 GPIO13
Pitch Servo Signal → ESP32 GPIO12
Roll Servo Signal  → ESP32 GPIO14
All Servo VCC      → 5V Power Supply
All Servo GND      → ESP32 GND + Power Supply GND
```

**Power:**
```
5V Power → ESP32 VIN pin
GND      → ESP32 GND
```

⚠️ **Important:** Common GND between ESP32 and power supply is required!

## Step 3: Configure & Upload (3 minutes)

1. **Download/Clone Repository**
   ```bash
   git clone [repository-url]
   cd 3AxisGimbalManual-Gryo
   ```

2. **Configure WiFi** (Optional - skip to use hotspot mode)
   ```bash
   cd esp32_firmware
   nano include/config.h
   ```
   
   Edit these lines:
   ```cpp
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   ```

3. **Upload Firmware**
   ```bash
   # Make sure ESP32 is connected via USB
   pio run --target upload
   ```

## Step 4: Connect & Test (2 minutes)

### First Boot

1. **Watch Serial Monitor** (optional)
   ```bash
   pio device monitor
   ```
   
   You should see:
   ```
   === ESP32 3-Axis Gimbal System ===
   Initializing MPU6050...
   MPU6050 initialized!
   Initializing servos...
   Servos initialized!
   Connecting to WiFi...
   [WiFi connected or Hotspot mode message]
   System ready!
   ```

2. **Connect to Gimbal**
   
   **Option A: WiFi Mode** (if configured)
   - Check serial monitor for IP address
   - Open browser: `http://[IP_ADDRESS]`
   
   **Option B: Hotspot Mode** (default)
   - Connect to WiFi network "Gimbal_AP"
   - Password: `gimbal123`
   - Open browser: `http://192.168.4.1`

3. **Test Controls**
   - Move the sliders - servos should respond
   - Switch to "Auto" mode
   - Tilt the gimbal - it should try to stabilize

## Congratulations! 🎉

Your gimbal is now operational!

## What's Next?

### Basic Usage

1. **Manual Control**
   - Use sliders to position gimbal
   - Click "Center Gimbal" to reset

2. **Auto Mode**
   - Set target position with "Auto Mode Target" sliders
   - Gimbal will maintain position using gyro

3. **Timed Moves**
   - Set duration and end position
   - Click "Start Timed Move"
   - Watch smooth automated movement

### Advanced Features

- **API Integration**: See [API.md](API.md)
- **FastAPI Backend**: See [README.md](../README.md#fastapi-backend-setup)
- **Mobile Apps**: API ready for mobile integration
- **Custom Moves**: Create preset sequences

## Common Issues

### ESP32 won't connect to WiFi
✅ **Solution:** It will automatically start hotspot. Connect to "Gimbal_AP"

### Servos not moving
✅ **Check:**
- Power supply connected?
- Servos getting 5V?
- Wiring correct?
- Check serial monitor for errors

### MPU6050 error
✅ **Check:**
- MPU6050 on 3.3V (not 5V!)
- SDA/SCL connections
- Sensor not damaged

### Web page won't load
✅ **Try:**
- Clear browser cache
- Different browser
- Check IP address in serial monitor
- Restart ESP32

## Pro Tips

💡 **Tip 1:** Save the IP address for quick access

💡 **Tip 2:** Adjust PID values in config.h for smoother auto mode

💡 **Tip 3:** Use external power for servos (not USB) for best performance

💡 **Tip 4:** Balance your gimbal mechanically before tuning software

💡 **Tip 5:** Start with light payloads, increase gradually

## Configuration Options

### WiFi Settings
```cpp
// config.h
#define WIFI_SSID "YourNetwork"
#define WIFI_PASSWORD "YourPassword"
#define HOTSPOT_SSID "Gimbal_AP"
#define HOTSPOT_PASSWORD "gimbal123"
```

### Servo Pins
```cpp
#define SERVO_PIN_YAW 13
#define SERVO_PIN_PITCH 12
#define SERVO_PIN_ROLL 14
```

### PID Tuning
```cpp
#define KP 2.0  // Increase for faster response
#define KI 0.5  // Increase to reduce steady-state error
#define KD 1.0  // Increase to reduce oscillation
```

### Update Rates
```cpp
#define SENSOR_UPDATE_RATE 10    // Faster = more responsive
#define SERVO_UPDATE_RATE 20     // Faster = smoother
#define WEBSOCKET_UPDATE_RATE 100 // Faster = more data
```

## Getting Help

- 📖 Full documentation in `/docs` folder
- 🔧 Hardware guide: [HARDWARE_SETUP.md](HARDWARE_SETUP.md)
- 🌐 API reference: [API.md](API.md)
- 🏗️ Architecture: [ARCHITECTURE.md](ARCHITECTURE.md)
- 📱 Deployment: [DEPLOYMENT.md](DEPLOYMENT.md)

## Example Projects

### Security Camera Mount
- Use auto mode for stabilization
- Add camera module
- Stream video while stabilized

### Timelapse Photography
- Program timed moves
- Smooth panning shots
- Sunrise/sunset tracking

### Robotics Platform
- Object tracking
- Sensor platform
- Interactive art installations

## Safety Reminder

⚠️ **Always:**
- Use appropriate power supply
- Check wiring before powering on
- Start with low-weight payloads
- Secure mounting to prevent falls
- Monitor temperature during operation

---

**Need more help?** See full documentation in `/docs` folder or check the [README](../README.md).

Happy building! 🚀
