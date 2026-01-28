# System Architecture Diagram

```
╔══════════════════════════════════════════════════════════════════════════╗
║                    ESP32 3-Axis Gimbal Control System                    ║
╚══════════════════════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────────────────────┐
│                          USER INTERFACES                                 │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐                  │
│  │              │  │              │  │              │                  │
│  │ Web Browser  │  │  Mobile App  │  │  REST API    │                  │
│  │  (Tailwind)  │  │   (Future)   │  │   Clients    │                  │
│  │              │  │              │  │              │                  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘                  │
│         │                 │                 │                           │
│         └─────────────────┴─────────────────┘                           │
│                           │                                              │
└───────────────────────────┼──────────────────────────────────────────────┘
                            │
                    HTTP/WebSocket
                            │
┌───────────────────────────▼──────────────────────────────────────────────┐
│                    OPTIONAL BACKEND SERVER                                │
├───────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  ┌──────────────────────────────────────────────────────────────┐        │
│  │                    FastAPI Backend                           │        │
│  │  ┌────────────────┐  ┌────────────────┐  ┌───────────────┐ │        │
│  │  │  REST API      │  │  WebSocket     │  │  Preset Mgmt  │ │        │
│  │  │  Endpoints     │  │  Server        │  │  & Storage    │ │        │
│  │  └────────────────┘  └────────────────┘  └───────────────┘ │        │
│  │  ┌────────────────────────────────────────────────────────┐ │        │
│  │  │         Auto-Generated Documentation                   │ │        │
│  │  │         (Swagger UI / ReDoc)                          │ │        │
│  │  └────────────────────────────────────────────────────────┘ │        │
│  └──────────────────────────────────────────────────────────────┘        │
│                                                                            │
└───────────────────────────────┬────────────────────────────────────────────┘
                                │
                        HTTP/WebSocket
                                │
┌───────────────────────────────▼────────────────────────────────────────────┐
│                         ESP32 DEVICE                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │                    APPLICATION LAYER                               │   │
│  │  ┌──────────────────────┐  ┌──────────────────────┐              │   │
│  │  │  Embedded Web Server │  │  WebSocket Server    │              │   │
│  │  │  (HTML/CSS/JS)       │  │  (Real-time Updates) │              │   │
│  │  └──────────────────────┘  └──────────────────────┘              │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │                      SERVICE LAYER                                 │   │
│  │  ┌──────────────┐  ┌─────────────┐  ┌──────────────────────────┐ │   │
│  │  │              │  │             │  │                          │ │   │
│  │  │ WiFi Manager │  │  Position   │  │    Auto-Stabilization    │ │   │
│  │  │   + Hotspot  │  │  Controller │  │    (PID Control)         │ │   │
│  │  │   Fallback   │  │             │  │                          │ │   │
│  │  │              │  │             │  │                          │ │   │
│  │  └──────────────┘  └─────────────┘  └──────────────────────────┘ │   │
│  │                                                                    │   │
│  │  ┌──────────────┐  ┌─────────────┐  ┌──────────────────────────┐ │   │
│  │  │              │  │             │  │                          │ │   │
│  │  │    Timed     │  │   Config    │  │      Preferences         │ │   │
│  │  │    Moves     │  │   Manager   │  │      Storage             │ │   │
│  │  │  Executor    │  │             │  │                          │ │   │
│  │  │              │  │             │  │                          │ │   │
│  │  └──────────────┘  └─────────────┘  └──────────────────────────┘ │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │              HARDWARE ABSTRACTION LAYER                            │   │
│  │  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐│   │
│  │  │                  │  │                  │  │                  ││   │
│  │  │  Servo Driver    │  │  MPU6050 Driver  │  │   WiFi Driver    ││   │
│  │  │  (3x PWM)        │  │  (I2C Sensor)    │  │   (Network)      ││   │
│  │  │                  │  │                  │  │                  ││   │
│  │  └──────────────────┘  └──────────────────┘  └──────────────────┘│   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────┬──────────────────────┬──────────────────────┬────────────────┘
              │                      │                      │
              │                      │                      │
┌─────────────▼──────────┐ ┌─────────▼──────────┐ ┌─────────▼──────────────┐
│                        │ │                    │ │                        │
│  3x SERVO MOTORS       │ │   MPU6050 SENSOR   │ │  WiFi / Hotspot        │
│  ┌─────┐ ┌─────┐      │ │                    │ │                        │
│  │ Yaw │ │Pitch│      │ │  ┌──────────────┐  │ │  ┌──────────────────┐  │
│  └─────┘ └─────┘      │ │  │ Gyroscope    │  │ │  │ SSID: Gimbal_AP  │  │
│          ┌─────┐      │ │  │ 3-axis       │  │ │  │ Pass: gimbal123  │  │
│          │Roll │      │ │  └──────────────┘  │ │  └──────────────────┘  │
│          └─────┘      │ │  ┌──────────────┐  │ │                        │
│  GPIO: 13, 12, 14     │ │  │Accelerometer│  │ │  Automatic Fallback    │
│  0-180° range         │ │  │ 3-axis       │  │ │  if WiFi fails         │
│                        │ │  └──────────────┘  │ │                        │
│                        │ │  I2C: GPIO21,22    │ │                        │
└────────────────────────┘ └────────────────────┘ └────────────────────────┘

╔══════════════════════════════════════════════════════════════════════════╗
║                         DATA FLOW                                         ║
╚══════════════════════════════════════════════════════════════════════════╝

MANUAL MODE:
  User Input → WebSocket → Position Controller → Servo Driver → Servos

AUTO MODE:
  MPU6050 → Sensor Manager → PID Controller → Position Controller → Servos
                            ↑
                       User Target

TIMED MOVE:
  User Command → Move Executor → Interpolation → Position Controller → Servos

SENSOR FEEDBACK (10Hz):
  MPU6050 → Sensor Manager → WebSocket → User Interface (Live Display)

╔══════════════════════════════════════════════════════════════════════════╗
║                      KEY FEATURES                                         ║
╚══════════════════════════════════════════════════════════════════════════╝

✓ Dual Operation Modes (Manual & Auto)
✓ WiFi with Hotspot Fallback
✓ Real-time WebSocket Updates
✓ PID-Controlled Stabilization
✓ Embedded Web Interface (Tailwind CSS)
✓ REST API for Remote Control
✓ Timed Movement Sequences
✓ Configuration Persistence
✓ Mobile-Ready Architecture
✓ Upgradeable & Extensible Design

╔══════════════════════════════════════════════════════════════════════════╗
║                    TECHNOLOGY STACK                                       ║
╚══════════════════════════════════════════════════════════════════════════╝

Firmware:     ESP32 Arduino Framework + PlatformIO
Frontend:     Tailwind CSS (Embedded)
Backend:      FastAPI (Python 3.8+)
Sensors:      Adafruit MPU6050 Library
Servos:       ESP32Servo Library
Network:      ESPAsyncWebServer + AsyncTCP
Data Format:  JSON (ArduinoJson)
Protocol:     HTTP REST + WebSocket
Architecture: DDD + SOC

╔══════════════════════════════════════════════════════════════════════════╗
║                      DEPLOYMENT OPTIONS                                   ║
╚══════════════════════════════════════════════════════════════════════════╝

1. STANDALONE
   └─ ESP32 Only (No Backend Needed)
      └─ Access via WiFi or Hotspot

2. WITH BACKEND
   └─ ESP32 + FastAPI Server
      └─ Advanced Features & Multi-Device

3. CLOUD DEPLOYMENT
   └─ ESP32 + Cloud Backend
      └─ Remote Access from Anywhere

4. DOCKER
   └─ Containerized Backend
      └─ Easy Scaling & Deployment

```

## Quick Reference

### Access Points
- **WiFi Mode**: `http://[ESP32_IP_ADDRESS]`
- **Hotspot Mode**: `http://192.168.4.1`
- **Backend API**: `http://[SERVER]:8000/docs`

### Default Credentials
- **Hotspot SSID**: `Gimbal_AP`
- **Hotspot Password**: `gimbal123`

### GPIO Pin Mapping
- **Yaw Servo**: GPIO 13
- **Pitch Servo**: GPIO 12
- **Roll Servo**: GPIO 14
- **MPU6050 SDA**: GPIO 21
- **MPU6050 SCL**: GPIO 22

### Operation Modes
- **Mode 0**: Manual (Direct Control)
- **Mode 1**: Auto (PID Stabilization)

### Update Rates
- **Sensors**: 100 Hz
- **Servos**: 50 Hz
- **WebSocket**: 10 Hz
