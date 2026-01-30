# Testing Guide

**Last Updated**: 2026-01-30  
**Version**: 1.0

This guide outlines testing strategies and procedures for the ESP32 3-Axis Gimbal Control System.

---

## Current Testing Status

**⚠️ IMPORTANT**: The project currently has **zero automated tests**. See [KnownIssues.MD #ISSUE-012](../KnownIssues.MD#issue-012-missing-automated-tests) for details.

All testing is currently **manual**. This document describes:
1. Current manual testing procedures
2. Planned automated testing strategy
3. How to contribute tests

---

## Table of Contents

1. [Manual Testing](#manual-testing)
2. [Planned Automated Testing](#planned-automated-testing)
3. [Test Environments](#test-environments)
4. [Test Coverage Goals](#test-coverage-goals)
5. [Contributing Tests](#contributing-tests)

---

## Manual Testing

### Hardware Tests

#### Test 1: Power-On Self Test (POST)

**Purpose**: Verify hardware initialization  
**Frequency**: Every boot

**Procedure**:
1. Power on device
2. Connect to serial monitor (115200 baud)
3. Observe POST output

**Expected Output**:
```
=== Power-On Self Test (POST) ===
Config System: OK
MPU6050 Sensor: OK (or FAILED - Manual mode only)
Servo Controllers: OK
=================================
```

**Pass Criteria**:
- Config system initializes
- Servos respond (run self-test to verify)
- Sensor detected (if available)

---

#### Test 2: Servo Movement Test

**Purpose**: Verify all servos respond correctly  
**Frequency**: After assembly, after firmware updates

**Procedure**:
1. Access web interface
2. Set mode to Manual
3. For each axis (Yaw, Pitch, Roll):
   - Move slider to 0°
   - Move slider to 90°
   - Move slider to 180°
   - Observe physical movement

**Pass Criteria**:
- All servos move smoothly
- No binding or mechanical issues
- Position matches commanded position (±5° tolerance)

---

#### Test 3: Sensor Reading Test

**Purpose**: Verify MPU6050 sensor provides valid data  
**Frequency**: After assembly, if auto mode issues occur

**Procedure**:
1. Place gimbal on level surface
2. Monitor serial console
3. Check sensor readings

**Expected Readings** (stationary, level):
- Accel X: ~0 m/s²
- Accel Y: ~0 m/s²
- Accel Z: ~9.8 m/s² (gravity)
- Gyro X: ~0 °/s
- Gyro Y: ~0 °/s
- Gyro Z: ~0 °/s

**Pass Criteria**:
- Readings are within ±1 m/s² for accel
- Readings are within ±5 °/s for gyro when stationary
- Readings change when gimbal is moved

---

### Software Tests

#### Test 4: Web Interface Connectivity

**Purpose**: Verify web server and WebSocket work  
**Frequency**: After firmware updates, after network changes

**Procedure**:
1. Connect to device WiFi or Hotspot
2. Open browser to device IP
3. Verify page loads
4. Check WebSocket status indicator

**Pass Criteria**:
- Web page loads within 3 seconds
- WebSocket connects (status shows "Connected")
- Real-time data updates visible

---

#### Test 5: API Endpoint Test

**Purpose**: Verify REST API endpoints respond correctly  
**Frequency**: After backend/firmware updates

**Test Cases**:

**5a. Get Status**
```bash
curl http://[DEVICE_IP]/api/status
```
Expected: JSON with current gimbal state

**5b. Set Position (Manual Mode)**
```bash
curl -X POST http://[DEVICE_IP]/api/position \
  -H "Content-Type: application/json" \
  -d '{"yaw": 120, "pitch": 90, "roll": 60}'
```
Expected: Position changes, returns `{"status":"ok"}`

**5c. Change Mode**
```bash
curl -X POST http://[DEVICE_IP]/api/mode \
  -H "Content-Type: application/json" \
  -d '{"mode": 1}'
```
Expected: Mode changes to Auto, returns success

**5d. Center Gimbal**
```bash
curl -X POST http://[DEVICE_IP]/api/center
```
Expected: Gimbal moves to 90°, 90°, 90°

**Pass Criteria**:
- All endpoints return HTTP 200
- Commands execute correctly
- Response JSON is valid

---

#### Test 6: Configuration Persistence

**Purpose**: Verify config saves and loads correctly  
**Frequency**: After ConfigManager changes

**Procedure**:
1. Change configuration via web interface:
   - Set PID values (e.g., Kp=3.0, Ki=0.6, Kd=1.2)
   - Set servo offsets
2. Save configuration
3. Power cycle device (reboot)
4. Check if configuration persisted

**Pass Criteria**:
- Configuration values persist across reboots
- No corruption of config.json
- All values match what was saved

---

#### Test 7: Auto Mode Stabilization

**Purpose**: Verify PID control maintains position  
**Frequency**: After PID changes, after sensor changes

**Procedure**:
1. Set mode to Auto
2. Set target position (90°, 90°, 90°)
3. Manually disturb gimbal (tilt it)
4. Observe recovery behavior

**Pass Criteria**:
- Gimbal returns to target position within 3 seconds
- No oscillation or overshoot
- Smooth movement

---

#### Test 8: Timed Move

**Purpose**: Verify smooth interpolated movements  
**Frequency**: After firmware updates

**Procedure**:
1. Set mode to Manual
2. Command timed move:
   - Duration: 5000ms (5 seconds)
   - Start: 0°, 0°, 0°
   - End: 180°, 180°, 180°
3. Observe movement

**Pass Criteria**:
- Movement takes exactly 5 seconds (±500ms)
- Movement is smooth, no jerking
- Ends at correct position

---

#### Test 9: Bluetooth Connectivity

**Purpose**: Verify BLE service advertises and accepts connections  
**Frequency**: After Bluetooth changes

**Procedure**:
1. Use BLE scanner app (e.g., nRF Connect)
2. Scan for devices
3. Connect to "ESP32_Gimbal"
4. Discover services/characteristics

**Pass Criteria**:
- Device appears in scan results
- Connection succeeds
- Service UUID `4fafc201-1fb5-459e-8fcc-c5c9c331914b` visible
- Characteristics readable/writable

**See**: [BENCH_TEST_GUIDE.md](BENCH_TEST_GUIDE.md) for detailed BLE testing

---

### Reliability Tests

#### Test 10: Long-Duration Stability

**Purpose**: Verify system stability over extended periods  
**Frequency**: Before production deployment

**Procedure**:
1. Set mode to Auto
2. Let system run for 24+ hours
3. Monitor for crashes, memory leaks, disconnects

**Pass Criteria**:
- No crashes or reboots
- WebSocket remains connected
- Memory usage stable (no leaks)
- Responsive to commands after 24 hours

---

#### Test 11: Power Interruption Recovery

**Purpose**: Verify system recovers from power loss  
**Frequency**: After ConfigManager changes

**Procedure**:
1. Configure system (PID, offsets, etc.)
2. Save configuration
3. Disconnect power mid-operation
4. Restore power
5. Check if config and operation resume

**Pass Criteria**:
- System boots normally after power loss
- Configuration intact (not corrupted)
- No data loss

---

#### Test 12: Network Failover

**Purpose**: Verify hotspot fallback works  
**Frequency**: After WiFiManager changes

**Procedure**:
1. Configure invalid WiFi credentials
2. Reboot device
3. Wait for WiFi connection timeout (10 seconds)
4. Verify hotspot activates

**Pass Criteria**:
- WiFi connection times out after 10 seconds
- Hotspot "Gimbal_AP" starts automatically
- Device accessible at 192.168.4.1
- LED indicates degraded mode if sensor missing

---

## Planned Automated Testing

### ESP32 Unit Tests (PlatformIO)

**Framework**: Unity Test Framework  
**Status**: Not implemented  
**Priority**: HIGH

#### Planned Test Suites:

**1. Domain Logic Tests** (`test_gimbal_controller`)
```cpp
void test_position_constraints() {
    // Test that positions are constrained to 0-180°
}

void test_timed_move_calculation() {
    // Test interpolation math
}

void test_center_position() {
    // Test center command sets 90°, 90°, 90°
}
```

**2. PID Controller Tests** (`test_pid_controller`)
```cpp
void test_pid_compute() {
    // Test PID output for known inputs
}

void test_pid_reset() {
    // Test integral reset
}

void test_pid_limits() {
    // Test output clamping
}
```

**3. Config Manager Tests** (`test_config_manager`)
```cpp
void test_save_load_config() {
    // Test round-trip save/load
}

void test_config_validation() {
    // Test invalid config rejection
}

void test_default_config() {
    // Test defaults when no file exists
}
```

**Setup**:
```ini
# platformio.ini
[env:native]
platform = native
test_framework = unity
build_flags = -DUNIT_TEST

[env:embedded]
platform = espressif32
board = esp32-s3-devkitc-1
test_framework = unity
```

**Run Tests**:
```bash
cd esp32_firmware
pio test -e native        # Run on host PC
pio test -e embedded      # Run on actual hardware
```

---

### Backend Unit Tests (pytest)

**Framework**: pytest  
**Status**: Not implemented  
**Priority**: HIGH

#### Planned Test Suites:

**1. API Endpoint Tests** (`test_api.py`)
```python
def test_get_status():
    response = client.get("/api/status")
    assert response.status_code == 200
    assert "position" in response.json()

def test_set_position_valid():
    response = client.post("/api/position", 
        json={"yaw": 90, "pitch": 90, "roll": 90})
    assert response.status_code == 200

def test_set_position_invalid_range():
    response = client.post("/api/position",
        json={"yaw": 200, "pitch": 90, "roll": 90})
    assert response.status_code == 400  # Bad request

def test_set_position_manual_mode_only():
    # Set mode to auto
    client.post("/api/mode", params={"mode": 1})
    # Try to set position (should fail)
    response = client.post("/api/position",
        json={"yaw": 90, "pitch": 90, "roll": 90})
    assert response.status_code == 400
```

**2. WebSocket Tests** (`test_websocket.py`)
```python
async def test_websocket_connection():
    async with client.websocket_connect("/ws") as ws:
        data = await ws.receive_json()
        assert "position" in data

async def test_websocket_broadcast():
    # Connect two clients
    # Send update
    # Verify both receive it
```

**Setup**:
```bash
cd backend
pip install pytest pytest-asyncio httpx

# Run tests
pytest tests/
pytest tests/ -v              # Verbose
pytest tests/ --cov=.         # With coverage
```

---

### Integration Tests

**Framework**: pytest + requests  
**Status**: Not implemented  
**Priority**: MEDIUM

**Purpose**: Test ESP32 + Backend integration

```python
def test_end_to_end_position_update():
    # Send command to backend
    # Backend forwards to ESP32
    # Verify ESP32 responds
    # Check WebSocket broadcasts update
```

---

### Load/Stress Tests

**Framework**: locust or artillery  
**Status**: Not implemented  
**Priority**: MEDIUM

**Purpose**: Test system under load

```python
# locustfile.py
from locust import HttpUser, task

class GimbalUser(HttpUser):
    @task
    def get_status(self):
        self.client.get("/api/status")
    
    @task(3)  # 3x more frequent
    def set_position(self):
        self.client.post("/api/position",
            json={"yaw": 90, "pitch": 90, "roll": 90})
```

**Run**:
```bash
locust -f locustfile.py --host=http://192.168.1.100
```

---

## Test Environments

### Local Development
- **Hardware**: ESP32-S3 DevKit on USB
- **Backend**: Local Python server
- **Network**: Direct USB serial + local WiFi

### CI/CD (Planned)
- **ESP32**: Build tests only (no hardware)
- **Backend**: Full unit + integration tests
- **Coverage**: Minimum 70% required

### Staging (Planned)
- **Hardware**: Dedicated test rig
- **Automated**: Nightly test runs
- **Environment**: Matches production

### Production
- **Monitoring**: Telemetry and logging
- **Alerts**: On failures
- **Rollback**: On critical issues

---

## Test Coverage Goals

| Component | Current | Goal |
|-----------|---------|------|
| Domain Logic | 0% | 80%+ |
| Services | 0% | 70%+ |
| API Endpoints | 0% | 90%+ |
| WebSocket | 0% | 80%+ |
| Configuration | 0% | 80%+ |
| **Overall** | **0%** | **75%+** |

---

## Contributing Tests

### Adding Unit Tests

1. Create test file in appropriate directory:
   - ESP32: `esp32_firmware/test/test_*.cpp`
   - Backend: `backend/tests/test_*.py`

2. Write test following existing patterns

3. Run tests locally:
   ```bash
   # ESP32
   cd esp32_firmware && pio test
   
   # Backend
   cd backend && pytest
   ```

4. Ensure tests pass before submitting PR

### Test Naming Convention

- Test functions: `test_<feature>_<scenario>`
- Test files: `test_<component>.cpp` or `test_<module>.py`

**Examples**:
```cpp
// Good
void test_pid_compute_with_zero_error()
void test_config_load_with_missing_file()

// Bad
void test1()
void pid_test()
```

### Test Documentation

Every test should have:
```cpp
/**
 * Test: PID controller computes correct output for proportional-only case
 * 
 * Given: Kp=2.0, Ki=0, Kd=0, error=10
 * When: PID compute is called
 * Then: Output should be 20.0 (2.0 * 10)
 */
void test_pid_compute_proportional_only() {
    // ...
}
```

---

## Test Checklist for PRs

Before submitting a PR, ensure:

- [ ] All existing tests pass
- [ ] New features have tests
- [ ] Tests cover edge cases
- [ ] Tests are documented
- [ ] Manual testing performed (see checklist below)
- [ ] No test coverage regressions

### Manual Testing Checklist

For PRs affecting:

**Hardware Control**:
- [ ] Servos move correctly
- [ ] No mechanical issues
- [ ] Safety limits enforced

**Networking**:
- [ ] Web interface loads
- [ ] WebSocket connects
- [ ] API endpoints respond

**Configuration**:
- [ ] Config saves correctly
- [ ] Config loads after reboot
- [ ] Invalid config rejected

**Stability**:
- [ ] No memory leaks (run for 1+ hour)
- [ ] No crashes
- [ ] Responsive under load

---

## Known Testing Issues

See [KnownIssues.MD #ISSUE-012](../KnownIssues.MD#issue-012-missing-automated-tests) for the comprehensive issue on lack of automated testing.

**Immediate Actions Needed**:
1. Set up PlatformIO test framework
2. Write unit tests for critical path (GimbalController, PIDController)
3. Set up pytest for backend
4. Add CI/CD pipeline for automated testing
5. Increase test coverage to 75%+

---

## Resources

- [PlatformIO Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/)
- [Unity Test Framework](http://www.throwtheswitch.org/unity)
- [pytest Documentation](https://docs.pytest.org/)
- [FastAPI Testing](https://fastapi.tiangolo.com/tutorial/testing/)
- [Bench Test Guide](BENCH_TEST_GUIDE.md) - Hardware testing procedures

---

**Version History**:
- v1.0 (2026-01-30): Initial testing documentation
