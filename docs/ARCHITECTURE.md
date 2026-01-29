# Architecture Documentation

## System Overview

The ESP32 3-Axis Gimbal Control System is designed using modern software engineering principles for maintainability, scalability, and upgradability.

## Design Principles

### Domain-Driven Design (DDD)

The system is organized around business domains:

1. **Hardware Control Domain**
   - Servo control
   - Sensor reading (MPU6050)
   - PWM signal generation

2. **Network Domain**
   - WiFi connection management
   - Hotspot fallback
   - WebSocket communication

3. **Control Domain**
   - Manual positioning
   - Auto-stabilization
   - Timed movements
   - Preset execution

4. **Configuration Domain**
   - Settings persistence
   - Runtime configuration
   - Calibration data

### Service-Oriented Components (SOC)

Each major functionality is encapsulated in loosely-coupled services:

```
┌─────────────────────────────────────────┐
│           Application Layer              │
│  ┌────────────┐  ┌──────────────────┐  │
│  │ Web Server │  │ WebSocket Server │  │
│  └────────────┘  └──────────────────┘  │
└─────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────┐
│           Service Layer                  │
│  ┌────────┐  ┌─────────┐  ┌──────────┐ │
│  │Position│  │  Auto   │  │  Timed   │ │
│  │Control │  │  Mode   │  │  Move    │ │
│  └────────┘  └─────────┘  └──────────┘ │
└─────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────┐
│         Hardware Abstraction Layer       │
│  ┌────────┐  ┌─────────┐  ┌──────────┐ │
│  │ Servo  │  │ MPU6050 │  │  WiFi    │ │
│  │ Driver │  │ Driver  │  │  Driver  │ │
│  └────────┘  └─────────┘  └──────────┘ │
└─────────────────────────────────────────┘
```

## Component Architecture

### ESP32 Firmware

```
esp32_firmware/
├── data/
│   ├── config.json           # Runtime configuration
│   └── index.html            # Single Page Application Frontend
├── src/
│   ├── Domain/
│   │   ├── GimbalController.cpp # Logic for movement and modes
│   │   └── PIDController.cpp    # Control loop logic
│   ├── Infrastructure/
│   │   └── SensorManager.cpp    # MPU6050 Hardware Interface
│   ├── Services/
│   │   ├── ConfigManager.cpp    # JSON/LittleFS Persistence
│   │   ├── WebManager.cpp       # WebServer & WebSocket
│   │   └── WiFiManager.cpp      # Network Connectivity
│   └── main.cpp              # Dependency Injection & Setup
├── include/
│   └── config.h              # Hardware Pinout & Constants
└── platformio.ini            # Build configuration
```

#### Key Components

1. **ConfigManager (Service)**
   - Handles loading/saving `config.json` via LittleFS.
   - Provides configuration object to other services.

2. **WiFiManager (Service)**
   - Connects to WiFi or creates Hotspot based on config.
   - Manages network state.

3. **WebManager (Service)**
   - Serves the frontend (`index.html`) from LittleFS.
   - Handles REST API (`/api/config`) and WebSocket communication.
   - Broadcasts real-time state to connected clients.

4. **GimbalController (Domain)**
   - **Core Logic**: Manages Manual, Auto, and Timed Move modes.
   - **PID Control**: Uses `PIDController` for stabilization.
   - **Servo Control**: smooths and writes to servos.

5. **SensorManager (Infrastructure)**
   - Abstraction over `Adafruit_MPU6050`.
   - Returns normalized sensor data.

### FastAPI Backend

```
backend/
├── main.py                   # FastAPI application
├── app/
│   ├── models.py             # Data models
│   ├── api/                  # API endpoints
│   └── services/             # Business logic
└── requirements.txt          # Dependencies
```

#### Responsibilities

1. **API Gateway**
   - Provides advanced API features
   - Implements authentication
   - Rate limiting and logging

2. **Preset Management**
   - Stores complex movement sequences
   - Manages preset library
   - Validates preset data

3. **Multi-Device Coordination**
   - Can control multiple gimbals
   - Synchronizes movements
   - Aggregates sensor data

## Data Flow

### Manual Mode

```
User Interface
     ↓
WebSocket/REST API
     ↓
Command Handler
     ↓
Position Controller
     ↓
Servo Driver
     ↓
Physical Servos
```

### Auto Mode

```
MPU6050 Sensor
     ↓
Sensor Manager
     ↓
PID Controller ←── User Target Position
     ↓
Position Controller
     ↓
Servo Driver
     ↓
Physical Servos
```

### Timed Move

```
User Command (start, duration, end position)
     ↓
Move Executor
     ↓
Interpolation Algorithm
     ↓
Position Updates (periodic)
     ↓
Servo Driver
     ↓
Physical Servos
```

## State Management

### Gimbal State

```cpp
struct GimbalState {
    int operationMode;        // MANUAL or AUTO
    GimbalPosition current;   // Current position
    GimbalPosition target;    // Target position
    GimbalPosition autoTarget; // Auto mode target
    bool isMoving;            // Movement in progress
    TimedMove activeMove;     // Current timed move
};
```

### Network State

```cpp
struct NetworkState {
    bool isWiFiConnected;
    bool isHotspotMode;
    IPAddress currentIP;
    int connectedClients;
};
```

## Communication Protocols

### WebSocket Protocol

**Message Format:**
```json
{
  "cmd": "command_name",
  "param1": value1,
  "param2": value2
}
```

**Supported Commands:**
- `setMode`: Change operation mode
- `setPosition`: Set manual position
- `setAutoTarget`: Set auto mode target
- `startTimedMove`: Begin timed movement

### REST API

Standard HTTP methods:
- `GET`: Retrieve data
- `POST`: Create/update
- `DELETE`: Remove

Response format: JSON

## Concurrency Model

### ESP32 (FreeRTOS)

The ESP32 firmware uses a single-loop architecture with timed tasks:

```cpp
void loop() {
    // Non-blocking task execution
    if (timeForSensorUpdate()) {
        updateSensors();
    }
    
    if (timeForServoUpdate()) {
        updateServos();
    }
    
    if (timeForWebSocketUpdate()) {
        sendWebSocketUpdate();
    }
    
    ws.cleanupClients();
}
```

Benefits:
- Predictable timing
- No race conditions
- Simple to understand

### FastAPI Backend (asyncio)

Asynchronous Python for concurrent connections:

```python
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    while True:
        data = await websocket.recv()
        # Process data
        await manager.broadcast(response)
```

Benefits:
- High concurrency
- Non-blocking I/O
- Efficient resource usage

## Security Architecture

### Current Implementation

- No authentication (suitable for local network)
- CORS enabled for web access
- No encryption (HTTP/WS)

### Production Recommendations

1. **Authentication Layer**
   ```python
   from fastapi.security import HTTPBearer
   
   security = HTTPBearer()
   
   @app.get("/api/status")
   async def get_status(credentials = Depends(security)):
       verify_token(credentials)
       # ...
   ```

2. **Encryption**
   - Use HTTPS (TLS/SSL)
   - Secure WebSocket (WSS)
   - Certificate management

3. **Authorization**
   - Role-based access control
   - Permission system
   - API key management

## Configuration Management

### Compile-Time Configuration

`config.h` contains constants:
- Pin assignments
- Network credentials
- PID parameters
- Update rates

### Runtime Configuration

ESP32 Preferences API:
- Operation mode
- Last position
- User settings

### Remote Configuration

Via web interface or API:
- All settings configurable
- Changes persist across reboots
- Backup/restore capability

## Extensibility

### Adding New Sensors

1. Create sensor interface:
```cpp
class SensorInterface {
public:
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual SensorData getData() = 0;
};
```

2. Implement for new sensor:
```cpp
class NewSensor : public SensorInterface {
    // Implementation
};
```

3. Register in main loop:
```cpp
NewSensor newSensor;
newSensor.begin();
// In loop: newSensor.update();
```

### Adding API Endpoints

ESP32:
```cpp
server.on("/api/new-feature", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Handle request
    request->send(200, "application/json", "{\"status\":\"ok\"}");
});
```

FastAPI:
```python
@app.get("/api/new-feature")
async def new_feature():
    return {"status": "ok"}
```

### Adding Movement Patterns

1. Define pattern interface:
```cpp
class MovementPattern {
public:
    virtual GimbalPosition getPosition(float time) = 0;
    virtual bool isComplete() = 0;
};
```

2. Implement specific pattern:
```cpp
class SineWavePattern : public MovementPattern {
    GimbalPosition getPosition(float time) {
        // Calculate sine wave position
    }
};
```

## Performance Considerations

### Update Rates

| Task | Rate | Priority |
|------|------|----------|
| Sensor reading | 100 Hz | High |
| Servo update | 50 Hz | High |
| PID calculation | 50 Hz | High |
| WebSocket send | 10 Hz | Medium |
| Web requests | As needed | Low |

### Memory Usage

- Firmware: ~200KB flash
- Runtime RAM: ~50KB
- WebSocket buffers: ~10KB
- Sensor buffers: ~2KB

### Network Performance

- WebSocket latency: <50ms
- REST API latency: <100ms
- Bandwidth: ~1KB/s (sensor data)

## Testing Strategy

### Unit Testing

- Test individual components
- Mock hardware interfaces
- Automated testing

### Integration Testing

- Test component interactions
- Hardware-in-the-loop testing
- End-to-end scenarios

### System Testing

- Full system validation
- Performance benchmarks
- Stress testing

## Deployment Architecture

### Standalone Mode

```
┌─────────────┐
│   Browser   │
└──────┬──────┘
       │ HTTP/WS
       │
┌──────▼──────┐
│    ESP32    │
│  (Complete  │
│   System)   │
└─────────────┘
```

### Backend Mode

```
┌─────────────┐
│   Browser   │
└──────┬──────┘
       │ HTTP/WS
       │
┌──────▼──────┐     ┌─────────────┐
│   FastAPI   │────→│    ESP32    │
│   Backend   │←────│   (Gimbal)  │
└─────────────┘     └─────────────┘
```

### Multi-Gimbal Mode

```
┌─────────────┐
│   Browser   │
└──────┬──────┘
       │
┌──────▼──────┐
│   FastAPI   │
│   Backend   │
└──┬────┬────┬┘
   │    │    │
   ▼    ▼    ▼
┌────┐┌────┐┌────┐
│ESP1││ESP2││ESP3│
└────┘└────┘└────┘
```

## Future Enhancements

### Planned Features

1. **Cloud Integration**
   - Remote access
   - Data logging
   - Analytics

2. **AI/ML**
   - Object tracking
   - Motion prediction
   - Auto-calibration

3. **Advanced Control**
   - Trajectory planning
   - Collision avoidance
   - Multi-gimbal sync

4. **Mobile Apps**
   - Native iOS app
   - Native Android app
   - Bluetooth control

### Scalability

The architecture supports:
- Multiple concurrent users
- Multiple gimbal devices
- Distributed deployment
- Cloud-based coordination

## References

- ESP32 Technical Reference: https://www.espressif.com/
- FastAPI Documentation: https://fastapi.tiangolo.com/
- DDD Principles: Domain-Driven Design by Eric Evans
- Clean Architecture: Robert C. Martin
