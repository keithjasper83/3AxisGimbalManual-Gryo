# ESP32 3-Axis Gimbal Control System

**📘 [Complete Deployment Guide](DEPLOYMENT.md)** - Step-by-step production deployment instructions with troubleshooting

**⚠️ IMPORTANT SECURITY NOTICE**: This software contains known security issues that must be addressed before production deployment. See [KnownIssues.MD](KnownIssues.MD) for a comprehensive list of issues and remediation steps.

**Critical Issues to Address**:
- 🔴 Change all default passwords (see [ISSUE-003](KnownIssues.MD#issue-003-hardcoded-default-passwords))
- 🔴 Restrict CORS origins in backend (see [ISSUE-001](KnownIssues.MD#issue-001-unrestricted-cors-in-backend-api))
- 🔴 Implement API authentication (see [ISSUE-002](KnownIssues.MD#issue-002-no-authentication-on-api-endpoints))
- 🔴 Enable Bluetooth pairing (see [ISSUE-010](KnownIssues.MD#issue-010-bluetooth-not-securedpaired))

**For a complete list of all 26 documented issues, see [KnownIssues.MD](KnownIssues.MD)**

---

A comprehensive, upgradeable ESP32-based 3-axis gimbal control system with web interface, REST API, and automatic stabilization using gyroscope and accelerometer sensors.

## 🌟 Features

- **Dual Operation Modes**
  - **Manual Mode**: Direct control via web interface or API
  - **Auto Mode**: Gyro-stabilized positioning to maintain user-defined angles

- **WiFi Connectivity with Fallback**
  - Automatic connection to configured WiFi network
  - Falls back to hotspot (Access Point) mode if connection fails
  - No internet required for operation

- **Bluetooth Low Energy (BLE)**
  - Direct device-to-device wireless control
  - Lower power consumption than WiFi
  - Mobile app integration ready
  - Real-time position and status updates

- **Phone Gyroscope Control**
  - Use your phone's motion sensors to control gimbal tilt
  - Intuitive tilt-to-control interface
  - Works on iOS and Android browsers
  - Real-time orientation feedback

- **Real-Time Web Interface**
  - Live gimbal control with sliders
  - Real-time sensor data visualization
  - Timed/pre-programmed moves
  - Responsive design with Tailwind CSS
  - WebSocket-based live updates
  - Phone gyroscope control integration

- **REST API**
  - Full control via HTTP endpoints
  - WebSocket support for real-time data
  - FastAPI backend with automatic documentation
  - Support for preset/timed moves

- **Advanced Features**
  - PID-controlled auto-stabilization
  - Smooth servo movements
  - Configuration persistence (LittleFS)
  - Pre-programmed timed moves
  - Configurable from web GUI
  - **Flat reference reset** - Set any position as new zero/flat
  - **Hardware button control** - Short press for flat reset, long press for self-test
  - **Self-test routine** - Automatic servo range verification
  - **Power-On Self Test (POST)** - Automatic hardware verification at startup

- **Hardware Resilience**
  - Graceful degradation when sensors fail
  - Continues operation in manual mode without gyroscope
  - Real-time hardware status monitoring
  - Visual warnings for missing hardware
  - **RGB LED Status Indicators** (ESP32-S3-N16R8):
    - 🔴 RED: Critical error (system halted)
    - 🟡 YELLOW: Sensor missing (manual mode only)
    - 🟢 GREEN (flashing): Boot in progress
    - 🟢 GREEN (solid): All systems operational

## 🏗️ Architecture

The system follows a modern, upgradeable architecture:

```
┌─────────────────┐
│   Web Browser   │
│  (Tailwind UI)  │
└────────┬────────┘
         │ HTTP/WebSocket
         │
┌────────▼────────┐
│  FastAPI Server │ ◄── Optional Backend
│   (Python API)  │     (for advanced features)
└────────┬────────┘
         │ HTTP/WebSocket
         │
┌────────▼────────┐
│   ESP32 Device  │
│  ┌──────────┐   │
│  │ Web Server│   │
│  ├──────────┤   │
│  │ MPU6050  │   │  ◄── Gyro + Accelerometer
│  ├──────────┤   │
│  │ 3x Servos│   │  ◄── Yaw, Pitch, Roll
│  └──────────┘   │
└─────────────────┘
```

### Design Principles

- **Domain-Driven Design (DDD)**: Clear separation of concerns
- **Service-Oriented Components (SOC)**: Modular, reusable components
- **Upgradeable**: Easy to extend with new features
- **Configurable**: All settings accessible via web GUI

## 📋 Hardware Requirements

### Required Components

1. **ESP32-S3 Development Board** (Recommended)
   - ESP32-S3-N16R8 (ESP32-S3 DevKitC-1) with onboard RGB LED
   - Minimum 512KB SRAM, 16MB Flash, 8MB PSRAM
   - USB-C connector for programming and power
   - Note: Original ESP32 boards are also supported but lack RGB LED status indicator

2. **MPU6050 Sensor**
   - 6-axis gyroscope and accelerometer
   - I2C interface

3. **3x Servo Motors**
   - Standard hobby servos (e.g., SG90, MG996R)
   - Operating voltage: 4.8V - 6V
   - Torque: Based on gimbal weight

4. **Power Supply**
   - 5V power supply for ESP32 and servos
   - Recommended: 5V 3A+ (depending on servo load)

5. **Additional Components**
   - Jumper wires
   - Breadboard or custom PCB
   - Gimbal mechanical structure

### Wiring Diagram

```
ESP32-S3        MPU6050
GPIO8 (SDA)  → SDA
GPIO9 (SCL)  → SCL
3.3V         → VCC
GND          → GND

ESP32-S3        Servos
GPIO13       → Yaw Servo Signal
GPIO12       → Pitch Servo Signal
GPIO14       → Roll Servo Signal
5V (External)→ Servo Power (All 3)
GND          → Servo Ground (All 3)

ESP32-S3        Button
GPIO15       → Button Pin
GND          → Button Ground

RGB LED (GPIO48) - Internal on ESP32-S3-N16R8
```

**📖 For complete GPIO pin details, see [GPIO Pin Reference Guide](docs/GPIO_PIN_REFERENCE.md)**  
**🔧 For bench testing procedures, see [Bench Test Guide](docs/BENCH_TEST_GUIDE.md)**

## 🚀 Installation

### ESP32 Firmware Setup

1. **Install PlatformIO**
   ```bash
   # Install PlatformIO Core
   pip install platformio
   ```

2. **Configure WiFi**
   Edit `esp32_firmware/include/config.h`:
   ```cpp
   #define WIFI_SSID "YourWiFiSSID"
   #define WIFI_PASSWORD "YourWiFiPassword"
   ```

3. **Upload Firmware & Filesystem**
   ```bash
   cd esp32_firmware
   # Upload the filesystem (HTML/CSS/Config)
   pio run --target uploadfs
   # Upload the firmware
   pio run --target upload
   # Monitor output
   pio device monitor
   ```

4. **Access Web Interface**
   - If connected to WiFi: Check serial monitor for IP address
   - If in hotspot mode: Connect to "Gimbal_AP" (password: gimbal123)
   - Open browser: `http://[IP_ADDRESS]` or `http://192.168.4.1` (hotspot)

### FastAPI Backend Setup (Optional)

The ESP32 can work standalone, but the FastAPI backend provides additional features:

1. **Install Dependencies**
   ```bash
   cd backend
   pip install -r requirements.txt
   ```

2. **Run Server**
   ```bash
   python main.py
   # or
   uvicorn main:app --host 0.0.0.0 --port 8000 --reload
   ```

3. **Access API Documentation**
   - Swagger UI: `http://localhost:8000/docs`
   - ReDoc: `http://localhost:8000/redoc`

## 📱 Usage

### Web Interface

1. **Connect to Gimbal**
   - WiFi mode: Connect to your network and navigate to ESP32's IP
   - Hotspot mode: Connect to "Gimbal_AP" and navigate to 192.168.4.1

2. **Manual Control**
   - Select "Manual" mode
   - Use sliders to control Yaw, Pitch, and Roll
   - Click "Center Gimbal" to return to neutral position

3. **Auto Mode**
   - Select "Auto" mode
   - Set target angles using "Auto Mode Target" sliders
   - Gimbal will automatically stabilize to maintain target position

4. **Timed Moves**
   - Set duration in seconds
   - Set end position for Yaw, Pitch, Roll
   - Click "Start Timed Move"
   - Gimbal smoothly moves from current to end position

### API Usage

#### Get Status
```bash
curl http://192.168.4.1/api/status
```

#### Set Manual Position
```bash
curl -X POST http://192.168.4.1/api/position \
  -H "Content-Type: application/json" \
  -d '{"yaw": 120, "pitch": 90, "roll": 60}'
```

#### Change Mode
```bash
# Manual mode
curl -X POST http://localhost:8000/api/mode?mode=0

# Auto mode
curl -X POST http://localhost:8000/api/mode?mode=1
```

#### WebSocket Connection
```javascript
const ws = new WebSocket('ws://192.168.4.1/ws');

ws.onopen = () => {
  console.log('Connected');
};

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  console.log('Gimbal data:', data);
};

// Send command
ws.send(JSON.stringify({
  cmd: 'setPosition',
  yaw: 90,
  pitch: 90,
  roll: 90
}));
```

## 🔧 Configuration

### ESP32 Configuration

All configuration is in `esp32_firmware/include/config.h`:

```cpp
// WiFi Settings
#define WIFI_SSID "YourNetwork"
#define WIFI_PASSWORD "YourPassword"
#define HOTSPOT_SSID "Gimbal_AP"
#define HOTSPOT_PASSWORD "gimbal123"

// Servo Pins
#define SERVO_PIN_YAW 13
#define SERVO_PIN_PITCH 12
#define SERVO_PIN_ROLL 14

// PID Parameters (Auto Mode)
#define KP 2.0  // Proportional gain
#define KI 0.5  // Integral gain
#define KD 1.0  // Derivative gain

// Update Rates
#define SENSOR_UPDATE_RATE 10    // ms
#define SERVO_UPDATE_RATE 20     // ms
#define WEBSOCKET_UPDATE_RATE 100 // ms
```

### Runtime Configuration

Settings persist across reboots using ESP32 Preferences:
- Operation mode (Manual/Auto)
- Last gimbal position
- Custom configurations added via web interface

## 📊 API Reference

### REST Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/status` | Get current gimbal status |
| POST | `/api/mode` | Set operation mode (0=Manual, 1=Auto) |
| POST | `/api/position` | Set manual position |
| POST | `/api/auto-target` | Set auto mode target |
| POST | `/api/timed-move` | Start timed move |
| POST | `/api/center` | Center gimbal |
| GET | `/api/presets` | List preset moves |
| POST | `/api/presets` | Create preset move |
| POST | `/api/presets/{name}/execute` | Execute preset |

### WebSocket Messages

#### From ESP32
```json
{
  "mode": 0,
  "position": {"yaw": 90, "pitch": 90, "roll": 90},
  "sensors": {
    "accel": {"x": 0.0, "y": 0.0, "z": 9.8},
    "gyro": {"x": 0.0, "y": 0.0, "z": 0.0}
  }
}
```

#### To ESP32
```json
{
  "cmd": "setPosition",
  "yaw": 120,
  "pitch": 90,
  "roll": 60
}
```

## 📱 Mobile App Integration

The system is ready for mobile app integration with multiple control options:

### Option 1: Phone Gyroscope Control (Web Interface)
Control the gimbal by tilting your phone - no app installation required!

1. Open the web interface on your mobile browser
2. Navigate to the **Phone Gyroscope Control** section
3. Tap **Enable** and grant motion sensor permission
4. Tilt your phone to control gimbal angles in real-time

**Supported Browsers**: Safari (iOS 13+), Chrome (Android), Firefox (Android)

### Option 2: Bluetooth BLE Control
Connect directly to the gimbal via Bluetooth Low Energy:

- **Device Name**: `ESP32_Gimbal`
- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Features**: Position control, mode switching, status notifications
- **Documentation**: See `/docs/BLUETOOTH_AND_PHONE_CONTROL.md`

**Advantages**:
- No WiFi required
- Lower latency (<50ms)
- Lower power consumption
- Direct device-to-device communication

### Option 3: Progressive Web App (PWA)
The web interface is mobile-responsive and can be saved as a PWA.

### Option 4: Native App Development
Use the REST API, WebSocket, or Bluetooth endpoints:
- React Native (Web Bluetooth or BLE libraries)
- Flutter (flutter_blue_plus)
- Native iOS/Android

Example code and full API documentation in `/docs/BLUETOOTH_AND_PHONE_CONTROL.md` and `/docs/API.md`.

## 🔐 Security Considerations

For production deployment:

1. **Change default passwords**
   ```cpp
   #define WIFI_PASSWORD "your-secure-password"
   #define HOTSPOT_PASSWORD "your-secure-hotspot-password"
   ```

2. **Enable authentication** (in FastAPI backend)
   - Uncomment authentication middleware
   - Configure JWT tokens

3. **Use HTTPS**
   - Configure SSL certificates
   - Use reverse proxy (nginx)

## 🛠️ Troubleshooting

### ESP32 Won't Connect to WiFi
- Check WiFi credentials in `config.h`
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check serial monitor for connection status
- System will automatically fall back to hotspot mode

### Servos Not Moving
- Check power supply (servos need sufficient current)
- Verify wiring connections
- Check servo pin definitions in `config.h`
- Monitor serial output for errors

### MPU6050 Not Found
- Check I2C connections (SDA/SCL)
- Verify MPU6050 power (3.3V)
- Check I2C address (default: 0x68)
- Install I2C scanner to verify device

### Web Interface Not Loading
- Check ESP32 IP address in serial monitor
- Ensure device and computer on same network
- Try accessing via hotspot mode
- Clear browser cache

## 🔄 Upgrade Path

The system is designed to be easily upgradeable:

### Hardware Upgrades
- Add IMU sensor (9-axis) for better stabilization
- Add camera module for FPV
- Add GPS for outdoor applications
- Use brushless gimbals for professional applications

### Software Upgrades
- Add AI/ML for object tracking
- Implement custom movement profiles
- Add multi-gimbal coordination
- Integrate with ROS for robotics

## 📚 Additional Resources

### Documentation
- **[GPIO Pin Reference](docs/GPIO_PIN_REFERENCE.md)** - Complete pin configuration and wiring details
- **[Bench Test Guide](docs/BENCH_TEST_GUIDE.md)** - Step-by-step testing procedures for BLE and servos
- **[Hardware Setup](docs/HARDWARE_SETUP.md)** - Detailed assembly and wiring instructions
- **[Bluetooth & Phone Control](docs/BLUETOOTH_AND_PHONE_CONTROL.md)** - BLE API and mobile control guide
- **[Architecture](docs/ARCHITECTURE.md)** - System design and code structure

### External Resources
- **PlatformIO**: https://platformio.org/
- **FastAPI**: https://fastapi.tiangolo.com/
- **ESP32 Documentation**: https://docs.espressif.com/
- **MPU6050 Datasheet**: [Link to datasheet]
- **Tailwind CSS**: https://tailwindcss.com/

## 🤝 Contributing

Contributions are welcome! Please read CONTRIBUTING.md for guidelines.

## 📄 License

MIT License - See LICENSE file for details

## 🙏 Acknowledgments

- ESP32 Arduino Core team
- FastAPI framework
- Adafruit sensor libraries
- Tailwind CSS team

## 📞 Support

For issues and questions:
- GitHub Issues: [Repository URL]
- Documentation: See `/docs` folder
- API Docs: `http://[device-ip]/docs`

---

**Version**: 1.0.0  
**Last Updated**: 2024