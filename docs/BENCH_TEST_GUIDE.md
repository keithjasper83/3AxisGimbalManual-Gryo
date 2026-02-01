# Bench Test Guide - ESP32 and Servo with BLE Mobile Control

## Purpose
This document provides a step-by-step procedure for bench testing the ESP32 3-Axis Gimbal system with a focus on mobile rotation control via Bluetooth Low Energy (BLE).

---

## Test Equipment Required

### Essential Hardware
- [ ] ESP32 Development Board
- [ ] USB cable (for programming and power)
- [ ] 1x Servo motor (SG90, MG90S, or similar)
- [ ] 5V Power Supply (1-2A minimum for single servo)
- [ ] Jumper wires (male-to-male and male-to-female)
- [ ] Breadboard (optional but recommended)

### Optional Hardware (for full testing)
- [ ] MPU6050 Gyroscope/Accelerometer sensor
- [ ] Push button (momentary)
- [ ] 2x Additional servos (for 3-axis testing)
- [ ] Multimeter (for voltage/continuity testing)

### Software Requirements
- [ ] PlatformIO installed (`pip install platformio`)
- [ ] Mobile device with Bluetooth capability (iOS 13+ or Android 5.0+)
- [ ] Web browser on mobile device (Safari, Chrome, or Firefox)

---

## Pre-Test Setup

### 1. Firmware Upload

```bash
# Navigate to firmware directory
cd esp32_firmware

# Configure WiFi (optional - can use hotspot mode)
# Edit include/config.h and set WIFI_SSID and WIFI_PASSWORD

# Upload filesystem (web interface)
pio run --target uploadfs

# Upload firmware
pio run --target upload

# Open serial monitor
pio device monitor
```

Expected serial output:
```
=== ESP32 3-Axis Gimbal System v1.2 ===

=== Power-On Self Test (POST) ===
Config System: OK
MPU6050 Sensor: FAILED (Manual mode only)  ← OK if sensor not connected
Servo Controllers: OK
=================================

Initializing WiFi...
[WiFi] Attempting to connect to: YourWiFiSSID
WiFi connected!
IP address: 192.168.1.XXX
Web server started!
Initializing Bluetooth...
Bluetooth BLE service started - Advertising as 'ESP32_Gimbal'
System Ready!
```

### 2. Initial Verification Checklist

- [ ] ESP32 powers on (LED lights up)
- [ ] Serial monitor shows boot messages
- [ ] POST (Power-On Self Test) completes
- [ ] WiFi connects OR hotspot mode activates
- [ ] BLE advertising starts ("Bluetooth BLE service started")
- [ ] "System Ready!" message appears

---

## Test Procedures

### Test 1: Power and Communication

**Objective**: Verify ESP32 is functioning and BLE is active

**Steps**:
1. Connect ESP32 via USB to computer
2. Open serial monitor (115200 baud)
3. Press RESET button on ESP32
4. Observe boot sequence

**Expected Results**:
- ✅ Clean boot with no errors (sensor fail is OK if not connected)
- ✅ BLE service initialized
- ✅ WiFi connected or hotspot active
- ✅ IP address displayed

**Troubleshooting**:
- If boot fails: Check USB connection and cable
- If BLE fails: Verify firmware has BLE libraries compiled
- If WiFi fails: System automatically falls back to hotspot mode

---

### Test 2: Bluetooth BLE Discovery

**Objective**: Verify mobile device can discover ESP32 BLE

**Steps**:
1. On mobile device, enable Bluetooth
2. Open Bluetooth settings
3. Scan for new devices
4. Look for "ESP32_Gimbal"

**Expected Results**:
- ✅ "ESP32_Gimbal" appears in available devices list
- ✅ Device shows as available (not paired/connected yet)

**Serial Monitor**: Should show no messages yet (connection happens at app level)

**Troubleshooting**:
- Device not found: Move closer to ESP32 (within 10 meters)
- Device not found: Restart ESP32 and try again
- Check serial monitor confirms "Bluetooth BLE service started"

---

### Test 3: Web Interface Access

**Objective**: Access gimbal control interface

**Steps**:

#### Option A: WiFi Mode
1. Connect mobile device to same WiFi network as ESP32
2. Note IP address from serial monitor (e.g., 192.168.1.100)
3. Open mobile browser
4. Navigate to `http://[IP_ADDRESS]`

#### Option B: Hotspot Mode
1. Connect mobile device to "Gimbal_AP" WiFi network
   - Password: `gimbal123`
2. Open mobile browser
3. Navigate to `http://192.168.4.1`

**Expected Results**:
- ✅ Web interface loads
- ✅ Shows gimbal controls (sliders for Yaw, Pitch, Roll)
- ✅ Shows current mode (Manual/Auto)
- ✅ Shows Bluetooth connection status

**Troubleshooting**:
- Page doesn't load: Check IP address
- Page doesn't load: Clear browser cache
- Page doesn't load: Try different browser

---

### Test 4: Single Servo Connection and Control

**Objective**: Test servo control with one servo (mobile rotation)

#### 4A: Hardware Connections

**Wiring Diagram**:
```
ESP32               Servo (Yaw)
GPIO 12  ────────→  Signal Wire (Orange/Yellow)

5V Power Supply
5V       ────────→  Servo Power (Red)
GND      ────────→  Servo Ground (Brown/Black)
         └────────→  ESP32 GND (COMMON GROUND!)
```

**Physical Setup**:
1. [ ] Connect servo signal wire to ESP32 GPIO 12
2. [ ] Connect 5V supply to servo power wire (Red)
3. [ ] Connect servo ground to power supply GND
4. [ ] **CRITICAL**: Connect ESP32 GND to power supply GND (common ground)
5. [ ] Double-check all connections
6. [ ] Verify servo is not mechanically blocked

#### 4B: Manual Control via Web Interface

**Steps**:
1. Access web interface (see Test 3)
2. Ensure "Manual" mode is selected
3. Find "Yaw" slider
4. Move slider slowly from left to right
5. Observe servo movement

**Expected Results**:
- ✅ Servo moves smoothly with slider
- ✅ Servo holds position when slider stops
- ✅ Servo responds to full range (0° to 180°)
- ✅ No jittering or erratic movement

**Serial Monitor Output**:
```
Manual Position: Yaw=45.0, Pitch=90.0, Roll=90.0
Manual Position: Yaw=90.0, Pitch=90.0, Roll=90.0
Manual Position: Yaw=135.0, Pitch=90.0, Roll=90.0
```

**Troubleshooting**:
- Servo doesn't move: Check signal wire connection to GPIO 12
- Servo jitters: Check power supply voltage (should be stable 5V)
- Servo jitters: Add 1000µF capacitor across servo power
- Servo weak: Check power supply current capacity (>1A)

---

### Test 5: BLE Control via Web Bluetooth API

**Objective**: Control servo via Bluetooth using web interface

**Steps**:
1. On web interface, find "Bluetooth Connection" section
2. Note status should show BLE device information
3. Use manual control sliders
4. Commands will be sent via WebSocket over WiFi/hotspot

**Expected Results**:
- ✅ Control works same as Test 4B
- ✅ Bluetooth status shows "Advertising" or "Connected"

**Note**: The web interface uses WebSocket over WiFi, not Web Bluetooth API directly. For true BLE control, see Test 6.

---

### Test 6: Direct BLE Control (Advanced)

**Objective**: Control servo directly via BLE without WiFi

**Requirements**:
- Mobile app that supports BLE (Web Bluetooth API in browser or native app)
- See `docs/BLUETOOTH_AND_PHONE_CONTROL.md` for API details

**BLE Service Details**:
- Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Position Characteristic (Write): `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Mode Characteristic (Write): `beb5483e-36e1-4688-b7f5-ea07361b26a9`
- Status Characteristic (Read/Notify): `beb5483e-36e1-4688-b7f5-ea07361b26aa`

**Test Procedure** (using Web Bluetooth API in Chrome):
1. Open browser on mobile device
2. Create test HTML page with Web Bluetooth code:

```html
<!DOCTYPE html>
<html>
<head><title>BLE Gimbal Test</title></head>
<body>
  <button id="connect">Connect BLE</button>
  <input type="range" id="yaw" min="0" max="180" value="90">
  <span id="yawValue">90</span>
  
  <script>
    let positionChar;
    
    document.getElementById('connect').addEventListener('click', async () => {
      const device = await navigator.bluetooth.requestDevice({
        filters: [{ name: 'ESP32_Gimbal' }],
        optionalServices: ['4fafc201-1fb5-459e-8fcc-c5c9c331914b']
      });
      
      const server = await device.gatt.connect();
      const service = await server.getPrimaryService('4fafc201-1fb5-459e-8fcc-c5c9c331914b');
      positionChar = await service.getCharacteristic('beb5483e-36e1-4688-b7f5-ea07361b26a8');
      
      alert('Connected!');
    });
    
    document.getElementById('yaw').addEventListener('input', async (e) => {
      const yaw = parseFloat(e.target.value);
      document.getElementById('yawValue').textContent = yaw;
      
      const buffer = new ArrayBuffer(12);
      const view = new DataView(buffer);
      view.setFloat32(0, yaw, true);    // Yaw
      view.setFloat32(4, 90.0, true);   // Pitch (fixed)
      view.setFloat32(8, 90.0, true);   // Roll (fixed)
      
      await positionChar.writeValue(buffer);
    });
  </script>
</body>
</html>
```

**Expected Results**:
- ✅ BLE device discovered
- ✅ Connection established
- ✅ Servo moves when slider adjusted
- ✅ Low latency (<100ms)

**Serial Monitor Output**:
```
BLE Client Connected
BLE Position: Yaw=45.0, Pitch=90.0, Roll=90.0
BLE Position: Yaw=90.0, Pitch=90.0, Roll=90.0
```

---

### Test 7: Mobile Phone Gyroscope Control

**Objective**: Control servo by tilting your phone

**Steps**:
1. Access web interface on mobile browser
2. Scroll to "Phone Gyroscope Control" section
3. Tap "Enable" button
4. Grant motion sensor permission (iOS 13+ will prompt)
5. Hold phone flat (screen facing up)
6. Slowly rotate phone left and right (yaw motion)
7. Observe servo following phone rotation

**Expected Results**:
- ✅ Permission granted successfully
- ✅ Status shows "Active: Receiving motion data..."
- ✅ Orientation values update in real-time
- ✅ Servo rotates as phone rotates
- ✅ Smooth tracking (not jittery)

**Serial Monitor Output**:
```
Position update: Yaw=75.0, Pitch=90.0, Roll=90.0
Position update: Yaw=85.0, Pitch=90.0, Roll=90.0
Position update: Yaw=95.0, Pitch=90.0, Roll=90.0
```

**Troubleshooting**:
- Permission denied: Check browser settings (iOS: Safari Settings → Motion & Orientation Access)
- Jittery movement: Hold phone more steadily
- No movement: Check that manual mode is active
- Not supported: Ensure using mobile device (not desktop)

---

### Test 8: MPU6050 Sensor (Optional)

**Objective**: Verify gyroscope sensor integration

**Prerequisites**:
- MPU6050 module connected to I2C pins

**Wiring**:
```
ESP32          MPU6050
GPIO 10  ───→  SDA (consecutive pins)
GPIO 11  ───→  SCL (consecutive pins)
3.3V     ───→  VCC
GND      ───→  GND
```

**Note**: GPIO 10 and 11 are physically consecutive for single header connection.

**Steps**:
1. Power off ESP32
2. Connect MPU6050 as shown above
3. Power on ESP32
4. Check serial monitor POST results

**Expected Results**:
- ✅ POST shows "MPU6050 Sensor: OK" (instead of FAILED)
- ✅ Auto mode becomes available
- ✅ Sensor readings appear in web interface

**Troubleshooting**:
- Sensor not detected: Check I2C wiring
- Sensor not detected: Verify 3.3V power (NOT 5V!)
- Sensor not detected: Try adding 4.7kΩ pull-up resistors on SDA/SCL lines

---

### Test 9: Three-Servo Configuration (Full System)

**Objective**: Test complete 3-axis gimbal

**Wiring**:
```
ESP32               Servos
GPIO 12  ────────→  Yaw Servo Signal    (consecutive pins)
GPIO 13  ────────→  Pitch Servo Signal  (consecutive pins)
GPIO 14  ────────→  Roll Servo Signal   (consecutive pins)

5V Power Supply (3A+)
5V       ────────→  All servo power wires (Red)
GND      ────────→  All servo ground wires (Brown/Black)
         └────────→  ESP32 GND (COMMON GROUND!)
```

**Note**: GPIO 12, 13, 14 are three consecutive pins for single header connection.

**Steps**:
1. Connect all three servos as shown
2. Use 3A+ power supply
3. Access web interface
4. Test each axis independently:
   - Move Yaw slider only
   - Move Pitch slider only
   - Move Roll slider only
5. Test combined movements
6. Test "Center Gimbal" button

**Expected Results**:
- ✅ All servos respond independently
- ✅ No interference between axes
- ✅ Smooth simultaneous movement
- ✅ Center button returns all to 90°

---

## Test Results Summary

### Test Checklist

| Test # | Test Name | Status | Notes |
|--------|-----------|--------|-------|
| 1 | Power and Communication | ☐ Pass / ☐ Fail | |
| 2 | BLE Discovery | ☐ Pass / ☐ Fail | |
| 3 | Web Interface Access | ☐ Pass / ☐ Fail | |
| 4 | Single Servo Control | ☐ Pass / ☐ Fail | |
| 5 | BLE Control via Web | ☐ Pass / ☐ Fail | |
| 6 | Direct BLE Control | ☐ Pass / ☐ Fail | |
| 7 | Phone Gyroscope Control | ☐ Pass / ☐ Fail | |
| 8 | MPU6050 Sensor | ☐ Pass / ☐ Fail / ☐ N/A | |
| 9 | Three-Servo Config | ☐ Pass / ☐ Fail / ☐ N/A | |

---

## Common Issues and Solutions

### Issue: ESP32 Won't Boot
- Check USB cable and connection
- Try different USB port
- Press RESET button on board
- Check power supply voltage

### Issue: Servo Makes Noise But Doesn't Move
- Servo may be mechanically blocked
- Check servo is not at physical limit
- Verify 5V power supply is adequate
- Try different servo

### Issue: BLE Not Advertising
- Restart ESP32
- Check serial monitor for "Bluetooth BLE service started"
- Ensure BLE is not disabled in code
- Try different mobile device

### Issue: Web Interface Slow or Laggy
- Check WiFi signal strength
- Reduce distance to ESP32
- Ensure ESP32 is not overheating
- Check power supply is stable

### Issue: Phone Gyroscope Permission Denied
- iOS: Go to Settings → Safari → Motion & Orientation Access
- Android: Grant browser location/sensor permissions
- Try different browser (Chrome/Firefox)

---

## Performance Benchmarks

### Expected Performance Metrics

| Metric | Expected Value | Acceptable Range |
|--------|---------------|------------------|
| BLE Latency | 30-50 ms | < 100 ms |
| WiFi WebSocket Latency | 50-100 ms | < 200 ms |
| Servo Response Time | 100-300 ms | < 500 ms |
| Phone Gyro Update Rate | 10 Hz | 5-20 Hz |
| Power Consumption (idle) | 200 mA | 150-300 mA |
| Power Consumption (active) | 500-1500 mA | Depends on servos |

---

## Safety Checklist

Before powering on:
- [ ] All connections verified correct
- [ ] No short circuits (check with multimeter)
- [ ] Power supply voltage correct (5V for servos, 3.3V for MPU6050)
- [ ] Common ground established
- [ ] Servo not mechanically blocked
- [ ] No loose wires that could short

---

## Next Steps After Successful Bench Test

1. **Document Your Results**: Note any issues or observations
2. **Mechanical Integration**: Mount servos in gimbal structure
3. **Calibration**: Adjust PID parameters for your specific setup
4. **Field Testing**: Test with actual payload (camera, etc.)
5. **Optimization**: Fine-tune settings based on performance

---

## Appendix: Quick Command Reference

### Serial Monitor Commands
```
# View connection status
- Watch for "System Ready!" message

# View BLE events
- "BLE Client Connected"
- "BLE Client Disconnected"
- "BLE Position: Yaw=X, Pitch=Y, Roll=Z"
```

### Web API Quick Test (curl)
```bash
# Get status
curl http://192.168.4.1/api/status

# Set position (manual mode)
curl -X POST http://192.168.4.1/api/position \
  -H "Content-Type: application/json" \
  -d '{"yaw": 90, "pitch": 90, "roll": 90}'

# Center gimbal
curl -X POST http://192.168.4.1/api/center
```

---

**Document Version**: 1.0  
**Date**: January 2026  
**Tested With**: ESP32 DevKit v1, Firmware v1.2
