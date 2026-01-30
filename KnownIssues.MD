# Known Issues and Technical Debt

**Last Updated**: 2026-01-30  
**Version**: 1.0  
**Status**: Living Document

This document tracks all known issues, security vulnerabilities, technical debt, and areas for improvement in the ESP32 3-Axis Gimbal Control System. Issues are categorized by severity and component.

---

## 📊 Issue Summary

| Severity | Total | Resolved | Remaining |
|----------|-------|----------|-----------|
| 🔴 Critical | 4 | 0 | 4 |
| 🟡 High | 7 | 0 | 7 |
| 🟠 Medium | 9 | 0 | 9 |
| 🟢 Low | 6 | 0 | 6 |
| **Total** | **26** | **0** | **26** |

---

## 🔴 Critical Security Issues

### ISSUE-001: Unrestricted CORS in Backend API
**Component**: `backend/main.py`  
**Severity**: 🔴 Critical  
**Type**: Security Vulnerability  
**Lines**: 13-19

**Description**:  
The FastAPI backend allows CORS from any origin (`allow_origins=["*"]`) with credentials enabled. This is a serious security vulnerability that allows any website to make authenticated requests to the API, enabling potential CSRF attacks and unauthorized access.

**Current Code**:
```python
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],              # ⚠️ Accepts requests from ANY origin
    allow_credentials=True,            # ⚠️ Allows cookies/auth headers
    allow_methods=["*"],
    allow_headers=["*"],
)
```

**Impact**:
- Any malicious website can make API calls to your gimbal
- CSRF attacks possible
- Credentials/session cookies exposed to any origin
- Production deployment security risk

**Remediation**:
```python
# Production-ready CORS configuration
allowed_origins = [
    "http://localhost:3000",           # Development frontend
    "http://192.168.1.100",            # ESP32 device IP
    "https://yourdomain.com",          # Production frontend
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=allowed_origins,     # Restrict to known origins
    allow_credentials=True,
    allow_methods=["GET", "POST", "PUT", "DELETE"],
    allow_headers=["Content-Type", "Authorization"],
)
```

**References**:
- OWASP CSRF Prevention: https://cheatsheetseries.owasp.org/cheatsheets/Cross-Site_Request_Forgery_Prevention_Cheat_Sheet.html
- MDN CORS: https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS

---

### ISSUE-002: No Authentication on API Endpoints
**Component**: `backend/main.py`, `esp32_firmware/src/Services/WebManager.cpp`  
**Severity**: 🔴 Critical  
**Type**: Security Vulnerability  
**Lines**: All API endpoints

**Description**:  
Neither the FastAPI backend nor the ESP32 web server implement any authentication mechanism. Anyone with network access can control the gimbal without authentication.

**Impact**:
- Unauthorized users can control gimbal movements
- Configuration can be changed without permission
- Denial of service possible (rapid movements, mode changes)
- Physical equipment damage risk from malicious control

**Affected Endpoints**:
- `POST /api/position` - Set manual position
- `POST /api/mode` - Change operation mode
- `POST /api/config` - Update configuration
- `POST /api/auto-target` - Set auto target
- `POST /api/timed-move` - Execute timed movements
- WebSocket `/ws` - Real-time control

**Remediation Priority**: HIGH  
**Recommended Solutions**:
1. **API Key Authentication** (Simplest for embedded)
   - Add API key header validation
   - Store hashed keys in config
   
2. **JWT Token Authentication** (Better security)
   - Implement OAuth2/JWT flow
   - Time-limited tokens
   
3. **mTLS** (Best for production)
   - Client certificate authentication
   - Hardware-level security

**Example Implementation**:
```python
# backend/main.py - API Key authentication
from fastapi import Security, HTTPException
from fastapi.security.api_key import APIKeyHeader

API_KEY_NAME = "X-API-Key"
api_key_header = APIKeyHeader(name=API_KEY_NAME, auto_error=False)

async def get_api_key(api_key: str = Security(api_key_header)):
    if api_key != settings.API_KEY:
        raise HTTPException(status_code=403, detail="Invalid API Key")
    return api_key

@app.post("/api/position", dependencies=[Depends(get_api_key)])
async def set_position(position: GimbalPosition):
    # Protected endpoint
    ...
```

---

### ISSUE-003: Hardcoded Default Passwords
**Component**: Multiple files  
**Severity**: 🔴 Critical  
**Type**: Security Vulnerability  
**Files**:
- `esp32_firmware/include/config.h` (line 8)
- `esp32_firmware/data/config.json` (line 5)
- `config.example.json` (line 15)
- `backend/main.py` (line 32)

**Description**:  
Default passwords are hardcoded throughout the codebase:
- Hotspot password: `"gimbal123"` (weak, publicly known)
- WiFi password placeholder: `"YourWiFiPassword"` (obvious placeholder)
- Config file password: `"CHANGE_ME_BEFORE_USE"` (better, but still default)

**Impact**:
- Users may deploy with default credentials
- Unauthorized access to hotspot network
- Control of gimbal from unauthorized devices

**Occurrences**:
```cpp
// config.h:8
#define HOTSPOT_PASSWORD "gimbal123"  // ⚠️ Weak default password

// config.json:5
"hotspot_password": "CHANGE_ME_BEFORE_USE"  // ⚠️ Still a default
```

**Remediation**:
1. **Force password change on first boot**
   - Detect default password at startup
   - Require user to set new password via serial console
   - Refuse to start hotspot with default password
   
2. **Generate random password**
   - Generate secure random password on first boot
   - Display on serial console
   - Store in config
   
3. **Better defaults**
   - Use ESP32 MAC address in default password
   - Example: `"Gimbal_" + last6digits_of_MAC`

**Recommended Fix**:
```cpp
// In WiFiManager::begin()
if (config.hotspot_password == "gimbal123" || 
    config.hotspot_password == "CHANGE_ME_BEFORE_USE") {
    Serial.println("ERROR: Default password detected!");
    Serial.println("Please set a secure hotspot password in config.json");
    Serial.println("System will not start hotspot with default password.");
    ledStatus.setStatus(LEDStatus::ERROR);
    return false;
}
```

---

### ISSUE-004: Passwords Stored in Plain Text
**Component**: `esp32_firmware/data/config.json`, `esp32_firmware/src/Services/ConfigManager.cpp`  
**Severity**: 🔴 Critical  
**Type**: Security Vulnerability  
**Files**:
- ConfigManager.cpp (lines 105, 107)
- config.json (lines 3, 5)

**Description**:  
WiFi and hotspot passwords are stored in plain text in the LittleFS filesystem. Anyone with physical access to the device can extract the filesystem and read all passwords.

**Current Implementation**:
```cpp
// ConfigManager.cpp:105-107
doc["wifi_password"] = config.wifi_password;        // Plain text
doc["hotspot_password"] = config.hotspot_password;  // Plain text
```

**Impact**:
- Physical access = password compromise
- Filesystem can be extracted via serial/USB
- Network credentials exposed
- Cross-device password reuse risk

**Remediation Options**:

**Option 1**: Basic Obfuscation (Better than nothing)
```cpp
// Simple XOR obfuscation
String obfuscate(String plain) {
    const uint8_t key = 0xA5;  // Simple key
    String result = "";
    for(char c : plain) {
        result += (char)(c ^ key);
    }
    return result;
}
```

**Option 2**: ESP32 Flash Encryption (Recommended)
- Enable flash encryption in menuconfig
- Passwords encrypted at hardware level
- Requires encrypted firmware updates
- Cannot be reversed without encryption key

**Option 3**: Hardware Security (Best)
- Use ESP32 eFuse for key storage
- Store passwords in secure element
- Hardware-backed encryption

**Note**: For WiFi, passwords MUST be decryptable to connect. For hotspot, WPA2 requires plain text password. This is a fundamental limitation, but we can make extraction more difficult.

---

## 🟡 High Severity Issues

### ISSUE-005: WebSocket Communication Not Authenticated
**Component**: `esp32_firmware/src/Services/WebManager.cpp`, `backend/main.py`  
**Severity**: 🟡 High  
**Type**: Security Vulnerability  
**Lines**: WebSocket handler functions

**Description**:  
WebSocket connections accept commands without any authentication. Once connected, any client can send control commands.

**Current Code**:
```cpp
// WebManager.cpp - No authentication check
void WebManager::handleWebSocketMessage(String message) {
    StaticJsonDocument<512> doc;
    deserializeJson(doc, message);
    
    // Commands executed without authentication
    if(doc["cmd"] == "setPosition") {
        _gimbalController.setManualPosition(...);
    }
}
```

**Impact**:
- Real-time unauthorized control
- Command injection possible
- No audit trail of who sent commands

**Remediation**:
- Implement WebSocket authentication tokens
- Verify token on connection and per-message
- Add rate limiting to prevent DoS

---

### ISSUE-006: No Input Validation on API Endpoints
**Component**: `backend/main.py`, `esp32_firmware/src/Services/WebManager.cpp`  
**Severity**: 🟡 High  
**Type**: Security & Reliability  
**Lines**: Multiple API handlers

**Description**:  
Most API endpoints don't validate input ranges before applying them to servos. Invalid values could damage hardware or cause unexpected behavior.

**Examples of Missing Validation**:
```python
# backend/main.py - No range validation
@app.post("/api/position")
async def set_position(position: GimbalPosition):
    gimbal_state["position"] = position.dict()  # ⚠️ No validation
```

**Potential Issues**:
- Servo values outside 0-180° range
- Negative PID parameters
- Extremely large duration values (integer overflow)
- Invalid mode values

**Expected Behavior**:
```python
@app.post("/api/position")
async def set_position(position: GimbalPosition):
    # Validate ranges
    if not (0 <= position.yaw <= 180):
        raise HTTPException(400, "Yaw must be 0-180")
    if not (0 <= position.pitch <= 180):
        raise HTTPException(400, "Pitch must be 0-180")
    if not (0 <= position.roll <= 180):
        raise HTTPException(400, "Roll must be 0-180")
    
    gimbal_state["position"] = position.dict()
```

**Remediation**:
- Add Pydantic validators for all models
- Add range checks in ESP32 handlers
- Reject invalid requests with clear error messages
- Add servo soft limits in config

---

### ISSUE-007: No Rate Limiting on API Endpoints
**Component**: `backend/main.py`, `esp32_firmware/src/Services/WebManager.cpp`  
**Severity**: 🟡 High  
**Type**: Security & Reliability  
**Lines**: All API endpoints

**Description**:  
API endpoints have no rate limiting. An attacker or buggy client could flood the API with requests, causing:
- Servo jitter from rapid position changes
- ESP32 resource exhaustion
- Denial of service

**Impact**:
- Physical damage from rapid servo movements
- System unresponsiveness
- Battery drain
- Thermal issues

**Remediation**:
```python
from slowapi import Limiter, _rate_limit_exceeded_handler
from slowapi.util import get_remote_address

limiter = Limiter(key_func=get_remote_address)
app.state.limiter = limiter

@app.post("/api/position")
@limiter.limit("10/minute")  # Max 10 position changes per minute
async def set_position(request: Request, position: GimbalPosition):
    ...
```

For ESP32:
```cpp
// Track last command time per endpoint
unsigned long lastPositionUpdate = 0;
const unsigned long MIN_UPDATE_INTERVAL = 100; // 100ms minimum

if (millis() - lastPositionUpdate < MIN_UPDATE_INTERVAL) {
    request->send(429, "application/json", 
        "{\"error\":\"Rate limit exceeded\"}");
    return;
}
```

---

### ISSUE-008: Missing Error Handling in Async Operations
**Component**: `backend/main.py`  
**Severity**: 🟡 High  
**Type**: Reliability  
**Lines**: 68-73, 106, 116, 124, etc.

**Description**:  
WebSocket broadcast and async operations don't handle exceptions. If a WebSocket client disconnects during broadcast, the exception could crash the broadcast loop.

**Current Code**:
```python
async def broadcast(self, message: str):
    for connection in self.active_connections:
        try:
            await connection.send_text(message)  # Could fail
        except:
            pass  # ⚠️ Silent failure - connection never removed
```

**Issues**:
- Dead connections accumulate in `active_connections` list
- Memory leak over time
- Broadcast gets slower as dead connections pile up
- No logging of failures

**Remediation**:
```python
async def broadcast(self, message: str):
    dead_connections = []
    for connection in self.active_connections:
        try:
            await connection.send_text(message)
        except Exception as e:
            logger.warning(f"Failed to send to client: {e}")
            dead_connections.append(connection)
    
    # Clean up dead connections
    for conn in dead_connections:
        self.disconnect(conn)
```

---

### ISSUE-009: No Firmware Version Verification
**Component**: `esp32_firmware/src/main.cpp`, `backend/main.py`  
**Severity**: 🟡 High  
**Type**: Compatibility & Maintainability  
**Lines**: N/A

**Description**:  
There's no mechanism to verify firmware and backend versions are compatible. API changes could cause unexpected behavior.

**Impact**:
- Silent failures from version mismatches
- Difficult to debug compatibility issues
- No upgrade path validation

**Remediation**:
1. Add version endpoint to ESP32
2. Backend checks ESP32 version on connection
3. Display warning if versions incompatible
4. Document breaking changes in CHANGELOG.md

---

### ISSUE-010: Bluetooth Not Secured/Paired
**Component**: `esp32_firmware/src/Services/BluetoothManager.cpp`  
**Severity**: 🟡 High  
**Type**: Security  
**Lines**: 60-108

**Description**:  
Bluetooth LE service is advertised openly without pairing or encryption. Any nearby device can discover and connect.

**Current Code**:
```cpp
// BluetoothManager.cpp:64
BLEDevice::init("ESP32_Gimbal");  // ⚠️ No security settings
```

**Impact**:
- Unauthorized Bluetooth connections
- Physical proximity = full control
- No way to revoke access

**Remediation**:
```cpp
// Enable pairing and encryption
BLEDevice::init("ESP32_Gimbal");
BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
BLESecurity *pSecurity = new BLESecurity();
pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
pSecurity->setCapability(ESP_IO_CAP_OUT);  // Display-only
pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
```

---

### ISSUE-011: No Backup/Restore for Configuration
**Component**: `esp32_firmware/src/Services/ConfigManager.cpp`  
**Severity**: 🟡 High  
**Type**: Usability  
**Lines**: N/A

**Description**:  
If config.json gets corrupted or misconfigured, there's no easy way to restore to a known-good state without reflashing.

**Impact**:
- Configuration corruption = device reset needed
- No way to rollback bad changes
- Lost calibration data

**Remediation**:
1. Create `/api/config/backup` endpoint
2. Create `/api/config/restore` endpoint
3. Store backup in separate file
4. Add config validation before applying

---

## 🟠 Medium Severity Issues

### ISSUE-012: Missing Automated Tests
**Component**: Entire project  
**Severity**: 🟠 Medium  
**Type**: Quality & Maintainability  
**Lines**: N/A

**Description**:  
The project has zero automated tests. All testing is manual, making regression testing difficult and time-consuming.

**Missing Test Coverage**:
- ❌ Unit tests for domain logic (GimbalController, PIDController)
- ❌ Integration tests for API endpoints
- ❌ Hardware simulation tests
- ❌ WebSocket communication tests
- ❌ Configuration management tests
- ❌ Bluetooth protocol tests

**Impact**:
- High risk of regressions
- Difficult to refactor with confidence
- Manual testing required for every change
- Longer development cycles

**Remediation**:
1. **ESP32 Unit Tests** - Use PlatformIO test framework
   ```ini
   # platformio.ini
   [env:native]
   platform = native
   test_framework = unity
   ```

2. **Backend Tests** - Use pytest
   ```python
   # tests/test_api.py
   def test_set_position_valid():
       response = client.post("/api/position", 
           json={"yaw": 90, "pitch": 90, "roll": 90})
       assert response.status_code == 200
   ```

3. **Integration Tests** - Test WebSocket communication
4. **CI/CD** - Automated test runs on commits

**Priority**: HIGH for production deployment

---

### ISSUE-013: Potential Integer Overflow in Timed Moves
**Component**: `esp32_firmware/src/Domain/GimbalController.cpp`  
**Severity**: 🟠 Medium  
**Type**: Reliability  
**Lines**: 70-85

**Description**:  
Timed move calculations use `unsigned long` for time tracking. Very long durations could theoretically overflow.

**Current Code**:
```cpp
unsigned long elapsed = millis() - _moveStartTime;  // Could overflow
if (elapsed >= _moveDuration) {
    _moveActive = false;
    ...
}
```

**Issues**:
- `millis()` overflows every ~49 days
- `_moveDuration` could be set extremely high
- No validation on duration input

**Remediation**:
1. Add maximum duration limit (e.g., 1 hour)
2. Use duration type checking
3. Handle `millis()` overflow properly

```cpp
// Add validation
bool GimbalController::startTimedMove(..., unsigned long duration) {
    const unsigned long MAX_DURATION = 3600000; // 1 hour max
    if (duration > MAX_DURATION) {
        Serial.println("Duration too long, max 1 hour");
        return false;
    }
    _moveDuration = duration;
    ...
}
```

---

### ISSUE-014: No Brownout Protection
**Component**: `esp32_firmware/src/main.cpp`  
**Severity**: 🟠 Medium  
**Type**: Hardware Safety  
**Lines**: N/A

**Description**:  
System doesn't monitor supply voltage. Low voltage can cause:
- Servo jitter
- ESP32 brownout resets
- Corrupted filesystem writes
- Lost calibration data

**Remediation**:
1. Monitor supply voltage using ADC
2. Reduce servo activity on low voltage
3. Prevent writes to config during brownout
4. Add voltage monitoring to status LED

```cpp
// Check power supply voltage
float getSupplyVoltage() {
    // Read voltage divider on ADC pin
    return analogRead(VOLTAGE_SENSE_PIN) * (3.3 / 4095.0) * DIVIDER_RATIO;
}

void loop() {
    float voltage = getSupplyVoltage();
    if (voltage < 4.5) {  // Below safe operating voltage
        ledStatus.setStatus(LEDStatus::WARNING);
        // Reduce servo activity
        gimbalController.setReducedPowerMode(true);
    }
}
```

---

### ISSUE-015: Mutex Lock Order Not Documented
**Component**: `esp32_firmware/src/Domain/GimbalController.cpp`, `esp32_firmware/src/Services/ConfigManager.cpp`  
**Severity**: 🟠 Medium  
**Type**: Thread Safety  
**Lines**: Multiple mutex operations

**Description**:  
Both GimbalController and ConfigManager use mutexes for thread safety. Lock order is not documented, raising risk of deadlock.

**Current Code Pattern**:
```cpp
// GimbalController.cpp:110-111
AppConfig config = _configManager.getConfig();  // Locks ConfigManager mutex
config.mode = mode;
_configManager.updateConfig(config);            // Locks ConfigManager mutex again

// GimbalController.cpp:115
xSemaphoreTake(_mutex, portMAX_DELAY);          // Locks GimbalController mutex
```

**Potential Deadlock Scenario**:
- Thread A: Holds ConfigManager mutex, waits for GimbalController mutex
- Thread B: Holds GimbalController mutex, waits for ConfigManager mutex
- Result: Deadlock

**Good News**: Current code carefully avoids holding gimbal mutex while calling config methods (line 110 comment: "WITHOUT holding gimbal mutex to avoid lock-order inversion")

**Remediation**:
1. Document lock order rules in header comments
2. Add lock order validation in debug builds
3. Consider lock-free alternatives where possible

```cpp
// ConfigManager.h
// LOCK ORDER RULES:
// 1. Always acquire ConfigManager mutex BEFORE GimbalController mutex
// 2. Never call ConfigManager methods while holding GimbalController mutex
// 3. Release all locks before calling methods that might lock
```

---

### ISSUE-016: No Logging Framework
**Component**: Entire project  
**Severity**: 🟠 Medium  
**Type**: Maintainability  
**Lines**: All Serial.println() calls

**Description**:  
Logging uses raw `Serial.println()` with no log levels, timestamps, or log management. Makes debugging production issues difficult.

**Current State**:
```cpp
Serial.println("BLE Client Connected");  // No timestamp, no level
Serial.println("Failed to open config file");  // No context
```

**Issues**:
- No way to filter logs by severity
- No timestamps on log messages
- No persistent log storage
- Serial must be connected to see logs

**Remediation**:
Use ESP_LOG or create simple logger:

```cpp
// SimpleLogger.h
enum LogLevel { ERROR, WARN, INFO, DEBUG };
class Logger {
    static void log(LogLevel level, const char* tag, const char* msg) {
        const char* levelStr[] = {"ERROR", "WARN", "INFO", "DEBUG"};
        Serial.printf("[%lu] [%s] [%s] %s\n", 
            millis(), levelStr[level], tag, msg);
    }
};

// Usage
Logger::log(INFO, "BLE", "Client Connected");
```

---

### ISSUE-017: PID Tuning Parameters Not Validated
**Component**: `esp32_firmware/src/Domain/PIDController.cpp`  
**Severity**: 🟠 Medium  
**Type**: Stability  
**Lines**: PID parameter setters

**Description**:  
PID parameters (Kp, Ki, Kd) can be set to any value via API without validation. Invalid values can cause instability.

**Dangerous Values**:
- Negative gains (system instability)
- Very large Ki (integral windup)
- Zero Kp with non-zero Ki/Kd (no proportional response)
- All zeros (no correction)

**Remediation**:
```cpp
bool PIDController::setTunings(float kp, float ki, float kd) {
    // Validate parameters
    if (kp < 0 || ki < 0 || kd < 0) {
        Serial.println("ERROR: PID gains must be non-negative");
        return false;
    }
    if (kp == 0 && (ki != 0 || kd != 0)) {
        Serial.println("WARN: Kp=0 with non-zero Ki/Kd may cause issues");
    }
    if (ki > 10.0) {
        Serial.println("WARN: Ki very large, integral windup possible");
    }
    
    _kp = kp;
    _ki = ki;
    _kd = kd;
    return true;
}
```

---

### ISSUE-018: No Watchdog Timer Implementation
**Component**: `esp32_firmware/src/main.cpp`  
**Severity**: 🟠 Medium  
**Type**: Reliability  
**Lines**: N/A

**Description**:  
No watchdog timer is configured. If the main loop hangs, the system won't recover automatically.

**Impact**:
- System hangs require physical reset
- No automatic recovery from crashes
- Poor reliability for unattended operation

**Remediation**:
```cpp
#include "esp_task_wdt.h"

void setup() {
    // Configure watchdog timer (10 second timeout)
    esp_task_wdt_init(10, true);
    esp_task_wdt_add(NULL);  // Add current task to WDT
    
    // Rest of setup...
}

void loop() {
    // Feed watchdog at start of each loop
    esp_task_wdt_reset();
    
    // Normal loop code...
}
```

---

### ISSUE-019: Filesystem Writes Not Atomic
**Component**: `esp32_firmware/src/Services/ConfigManager.cpp`  
**Severity**: 🟠 Medium  
**Type**: Data Integrity  
**Lines**: 96-115

**Description**:  
Configuration file writes are not atomic. Power loss during write corrupts config.json.

**Current Code**:
```cpp
File file = LittleFS.open(_filename, "w");  // Truncates file first
if (!file) return false;
serializeJson(doc, file);  // ⚠️ If power lost here, file is corrupt
file.close();
```

**Impact**:
- Power loss during save = corrupted config
- Device boots with corrupt config
- Lost calibration data

**Remediation**:
```cpp
// Write-rename pattern for atomic updates
bool ConfigManager::_saveConfigInternal() {
    const char* tempFile = "/config.tmp";
    
    // Write to temporary file first
    File file = LittleFS.open(tempFile, "w");
    if (!file) return false;
    
    size_t bytesWritten = serializeJson(doc, file);
    file.close();
    
    if (bytesWritten == 0) {
        LittleFS.remove(tempFile);
        return false;
    }
    
    // Atomic rename
    LittleFS.remove(_filename);
    return LittleFS.rename(tempFile, _filename);
}
```

---

### ISSUE-020: No Circuit Breaker Pattern for External Services
**Component**: `backend/main.py`  
**Severity**: 🟠 Medium  
**Type**: Resilience  
**Lines**: N/A

**Description**:  
Backend doesn't implement circuit breaker pattern. If backend repeatedly fails to connect to ESP32, it keeps trying indefinitely.

**Remediation**:
Implement circuit breaker to prevent cascading failures:

```python
from circuitbreaker import circuit

@circuit(failure_threshold=5, recovery_timeout=60)
async def send_to_esp32(command):
    # If this fails 5 times, circuit opens
    # After 60 seconds, circuit half-opens and retries
    ...
```

---

## 🟢 Low Severity Issues

### ISSUE-021: Typo in Repository Name
**Component**: Repository name  
**Severity**: 🟢 Low  
**Type**: Documentation  
**Lines**: N/A

**Description**:  
Repository name is `3AxisGimbalManual-Gryo` but should be `3AxisGimbalManual-Gyro` (Gyro, not Gryo).

**Impact**:
- Confusing for users
- Unprofessional appearance
- SEO impact

**Remediation**:  
Rename repository through GitHub settings. Update all documentation references.

**Note**: Renaming a repository breaks existing clones and links. Consider carefully.

---

### ISSUE-022: Missing CALIBRATION.md Document
**Component**: Documentation  
**Severity**: 🟢 Low  
**Type**: Documentation  
**Files**: Referenced but missing

**Description**:  
Several files reference a `CALIBRATION.md` document that doesn't exist:
- config.json references calibration values
- README mentions calibration process
- No step-by-step calibration guide exists

**Impact**:
- Users don't know how to calibrate servos
- Improper calibration = poor performance
- Support burden

**Remediation**:  
Create `/docs/CALIBRATION.md` with:
1. Servo centering procedure
2. Offset adjustment steps
3. PID tuning guide
4. Flat reference setup
5. IMU calibration (if needed)

---

### ISSUE-023: Incomplete .gitignore
**Component**: `.gitignore`  
**Severity**: 🟢 Low  
**Type**: Repository Hygiene  
**Lines**: Various

**Description**:  
`.gitignore` may be missing common build artifacts and IDE files.

**Missing Patterns**:
```gitignore
# PlatformIO
.pio/
.pioenvs/
.piolibdeps/

# Build artifacts
*.o
*.a
*.elf
*.bin

# IDE files
.vscode/
*.code-workspace
.idea/

# Python
__pycache__/
*.pyc
.pytest_cache/
venv/
.env

# macOS
.DS_Store

# Temporary files
*.tmp
*.bak
*~
```

**Remediation**:  
Audit and update `.gitignore` file.

---

### ISSUE-024: Magic Numbers in Code
**Component**: Multiple source files  
**Severity**: 🟢 Low  
**Type**: Code Quality  
**Lines**: Various

**Description**:  
Magic numbers appear throughout codebase without named constants.

**Examples**:
```cpp
// GimbalController.cpp:89
_currentPos.yaw += (_targetPos.yaw - _currentPos.yaw) * 0.1;  // What is 0.1?

// main.cpp:155
if (currentTime - lastButtonCheck >= 10) {  // Why 10?

// BluetoothManager.cpp:26
if (value.length() == 12) {  // Why 12?
```

**Remediation**:
```cpp
// Define named constants
const float SERVO_SMOOTHING_FACTOR = 0.1;  // Lower = smoother movement
const unsigned long BUTTON_CHECK_INTERVAL_MS = 10;
const size_t BLE_POSITION_DATA_SIZE = 12;  // 3 floats * 4 bytes
```

---

### ISSUE-025: Inconsistent Code Formatting
**Component**: Multiple source files  
**Severity**: 🟢 Low  
**Type**: Code Quality  
**Lines**: Various

**Description**:  
Code formatting is inconsistent across files:
- Mixed brace styles
- Inconsistent indentation
- Variable naming conventions vary

**Examples**:
- Some files use `_privateVar`, others `m_privateVar`
- Mix of `camelCase` and `snake_case`
- Inconsistent spacing around operators

**Remediation**:
1. Choose a style guide (e.g., Google C++ Style)
2. Add `.clang-format` configuration
3. Run formatter on all files
4. Add formatting check to CI

**Example `.clang-format`**:
```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
```

---

### ISSUE-026: Missing API Rate Limiting Documentation
**Component**: Documentation  
**Severity**: 🟢 Low  
**Type**: Documentation  
**Lines**: N/A

**Description**:  
API documentation doesn't mention any rate limits or best practices for API usage.

**Impact**:
- Users don't know usage limits
- Accidental API abuse
- Poor client implementations

**Remediation**:  
Add to `/docs/API.md`:
```markdown
## Rate Limits

| Endpoint | Limit | Window |
|----------|-------|--------|
| /api/position | 10 req/min | Per IP |
| /api/mode | 5 req/min | Per IP |
| /api/config | 2 req/min | Per IP |
| WebSocket messages | 20 msg/sec | Per connection |

**Best Practices**:
- Cache position reads when possible
- Batch position updates
- Use WebSocket for high-frequency updates
- Implement exponential backoff on errors
```

---

## 📋 Architecture & Technical Debt

### DEBT-001: Lack of Dependency Injection
**Component**: ESP32 firmware  
**Type**: Architecture  

**Description**:  
Components are tightly coupled. Testing and mocking are difficult.

**Remediation**:  
Consider using dependency injection pattern for better testability.

---

### DEBT-002: Global State in Backend
**Component**: `backend/main.py`  
**Type**: Architecture  
**Lines**: 43-54

**Description**:  
Backend uses global dictionaries for state. Not scalable, not thread-safe.

**Remediation**:  
- Use proper database (SQLite, PostgreSQL)
- Implement state management class
- Add proper async locks

---

### DEBT-003: No Logging to File
**Component**: Both ESP32 and backend  
**Type**: Observability  

**Description**:  
No persistent logs. Debugging production issues is difficult.

**Remediation**:
- ESP32: Log to SD card or LittleFS (with rotation)
- Backend: Use Python logging to file
- Add log rotation
- Consider centralized logging (syslog, CloudWatch)

---

## 🔄 Issue Workflow

### Reporting New Issues
1. Add issue to this document
2. Assign severity and type
3. Link to relevant code locations
4. Add to summary table
5. Update issue placeholders in code

### Resolving Issues
1. Fix the issue
2. Add tests if applicable
3. Update status to "Resolved"
4. Document resolution in commit
5. Remove code placeholders
6. Update summary table

---

## 🏷️ Issue Categories

- **Security Vulnerability**: Could be exploited maliciously
- **Reliability**: Could cause crashes or data loss
- **Performance**: Impacts system responsiveness
- **Usability**: Affects user experience
- **Maintainability**: Makes code harder to maintain
- **Documentation**: Missing or incorrect docs
- **Code Quality**: Style, readability, best practices

---

## 📚 References

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [OWASP IoT Top 10](https://owasp.org/www-project-internet-of-things/)
- [CWE Top 25](https://cwe.mitre.org/top25/)
- [ESP32 Security Best Practices](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/security.html)

---

**Document Maintenance**: This document should be updated whenever:
- New issues are discovered
- Issues are resolved
- Security vulnerabilities are found
- Architecture changes are made
- Code reviews identify problems

**Version History**:
- v1.0 (2026-01-30): Initial comprehensive review
