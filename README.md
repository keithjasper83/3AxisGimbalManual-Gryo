# ESP32 3-Axis Gimbal Control System

A comprehensive, upgradeable ESP32-based 3-axis gimbal control system with web interface, REST API, and automatic stabilization using gyroscope and accelerometer sensors.

## 🌟 Features

- **Dual Operation Modes**
  - **Manual Mode**: Direct control via web interface or API
  - **Auto Mode**: Gyro-stabilized positioning to maintain user-defined angles

- **WiFi Connectivity with Fallback**
  - Automatic connection to configured WiFi network
  - Falls back to hotspot (Access Point) mode if connection fails
  - No internet required for operation

- **Real-Time Web Interface**
  - Live gimbal control with sliders
  - Real-time sensor data visualization
  - Timed/pre-programmed moves
  - Responsive design with Tailwind CSS
  - WebSocket-based live updates

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

1. **ESP32 Development Board**
   - Any ESP32 board (ESP32-WROOM-32, ESP32-DevKit, etc.)
   - Minimum 520KB SRAM, 4MB Flash

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
ESP32          MPU6050
GPIO21 (SDA) → SDA
GPIO22 (SCL) → SCL
3.3V         → VCC
GND          → GND

ESP32          Servos
GPIO13       → Yaw Servo Signal
GPIO12       → Pitch Servo Signal
GPIO14       → Roll Servo Signal
5V           → Servo VCC (all 3)
GND          → Servo GND (all 3)
```

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

The system is ready for mobile app integration:

### Option 1: Progressive Web App (PWA)
The web interface is mobile-responsive and can be saved as a PWA.

### Option 2: Native App
Use the REST API and WebSocket endpoints:
- React Native
- Flutter
- Native iOS/Android

Example endpoints are fully documented in `/docs/API.md`.

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