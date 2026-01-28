# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.1] - 2024-01-28

### Security
- Updated FastAPI from 0.104.1 to 0.109.1 to fix ReDoS vulnerability (CVE)
- Updated python-multipart from 0.0.6 to 0.0.22 to fix multiple vulnerabilities:
  - Arbitrary file write vulnerability
  - Denial of service (DoS) vulnerability
  - Content-Type Header ReDoS vulnerability

## [1.0.0] - 2024-01-28

### Added
- Initial release of ESP32 3-Axis Gimbal Control System
- ESP32 firmware with PlatformIO support
- MPU6050 gyroscope and accelerometer integration
- 3-axis servo control (Yaw, Pitch, Roll)
- Dual operation modes (Manual and Auto)
- WiFi connectivity with automatic hotspot fallback
- Web-based control interface with Tailwind CSS
- Real-time WebSocket communication
- REST API endpoints for gimbal control
- FastAPI backend server (optional)
- PID-controlled auto-stabilization
- Timed movement execution
- Configuration persistence using ESP32 Preferences
- Comprehensive documentation:
  - README.md with full feature list
  - Hardware setup guide
  - API documentation
  - Architecture documentation
  - Deployment guide
  - Quick start guide
- Example configuration files
- MIT License

### Features

#### ESP32 Firmware
- WiFi connection with configurable credentials
- Hotspot mode (SSID: Gimbal_AP, Password: gimbal123)
- MPU6050 sensor reading at 100Hz
- Servo control at 50Hz
- WebSocket updates at 10Hz
- Manual position control
- Auto-stabilization using PID control
- Smooth servo movements with interpolation
- Timed moves with linear interpolation
- Configuration storage and retrieval

#### Web Interface
- Responsive design with Tailwind CSS
- Real-time position control sliders
- Mode switching (Manual/Auto)
- Auto mode target position setting
- Timed move configuration
- Live sensor data display
- WebSocket connection status
- Center gimbal button

#### Backend API
- FastAPI server with automatic docs
- RESTful API endpoints
- WebSocket support
- Preset move management
- Multi-gimbal support (architecture ready)
- CORS configuration
- Health check endpoint

#### Documentation
- Complete README with installation instructions
- Hardware wiring diagrams
- API reference with code examples
- Architecture and design documentation
- Deployment guide (standalone, cloud, Docker)
- Quick start guide (15-minute setup)
- Contributing guidelines

### Technical Details

#### Dependencies
- ESP32 Arduino framework
- PlatformIO build system
- Adafruit MPU6050 library
- ESP32Servo library
- ESPAsyncWebServer
- ArduinoJson
- FastAPI (backend)
- Uvicorn (backend)

#### Configuration
- WiFi SSID and password
- Servo pin assignments (GPIO 13, 12, 14)
- MPU6050 I2C pins (GPIO 21, 22)
- PID parameters (Kp=2.0, Ki=0.5, Kd=1.0)
- Update rates (configurable)
- Servo limits (0-180 degrees)

### Architecture
- Domain-Driven Design (DDD)
- Service-Oriented Components (SOC)
- Modular, upgradeable design
- Clean separation of concerns
- Hardware abstraction layer

### Supported Platforms
- ESP32 (all variants)
- Linux (backend)
- macOS (backend)
- Windows (backend)
- Docker (backend)

### Browser Support
- Chrome/Edge (latest)
- Firefox (latest)
- Safari (latest)
- Mobile browsers

## [Unreleased]

### Planned Features
- Mobile app (React Native/Flutter)
- Object tracking with camera
- GPS integration for outdoor use
- Advanced preset patterns
- Cloud connectivity
- User authentication
- Multi-gimbal synchronization
- Advanced PID auto-tuning
- Movement recording and playback
- Database integration for presets
- Bluetooth control option
- Battery monitoring
- Over-the-air (OTA) updates
- Web-based configuration editor
- Advanced telemetry and logging

---

## Version History

- **1.0.0** (2024-01-28): Initial public release
