# Gimbal Calibration Guide

**Last Updated**: 2026-01-30  
**Version**: 1.0

This guide provides step-by-step instructions for calibrating your ESP32 3-Axis Gimbal Control System to achieve optimal performance.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Servo Centering](#servo-centering)
3. [Servo Offset Adjustment](#servo-offset-adjustment)
4. [Flat Reference Setup](#flat-reference-setup)
5. [PID Tuning](#pid-tuning)
6. [IMU Calibration](#imu-calibration)
7. [Troubleshooting](#troubleshooting)

---

## Prerequisites

Before calibrating, ensure:
- ✅ Hardware is properly assembled (see [HARDWARE_SETUP.md](HARDWARE_SETUP.md))
- ✅ Firmware is uploaded and running
- ✅ You can access the web interface
- ✅ MPU6050 sensor is detected (check serial monitor)
- ✅ All three servos respond to commands

**Tools Needed**:
- Level or spirit level app on smartphone
- Ruler or measuring tape
- Screwdriver for mechanical adjustments
- Serial monitor for debugging

---

## 1. Servo Centering

Servo centering ensures that when the gimbal is commanded to 90° (center position), all servos are at their mechanical center.

### Step 1: Power On and Connect

1. Power on the ESP32 gimbal system
2. Connect to the web interface
3. Set mode to **Manual**

### Step 2: Command Center Position

1. In the web interface, click **"Center Gimbal"** button
2. All axes should move to 90° (center position)
3. Alternatively, use API:
   ```bash
   curl -X POST http://[DEVICE_IP]/api/position \
     -H "Content-Type: application/json" \
     -d '{"yaw": 90, "pitch": 90, "roll": 90}'
   ```

### Step 3: Check Mechanical Alignment

**Expected Results**:
- **Yaw Axis**: Camera/platform should face straight forward
- **Pitch Axis**: Camera/platform should be level (0° tilt)
- **Roll Axis**: Camera/platform should be level (no rotation)

**If servos are NOT centered**:
- Check servo horn installation (may need to be repositioned)
- Mechanical adjustment may be needed
- Proceed to Servo Offset Adjustment

---

## 2. Servo Offset Adjustment

Servo offsets compensate for mechanical imperfections and servo horn positioning errors.

### What Are Offsets?

Offsets are added to the commanded position before sending to the servo:
```
Physical Position = Commanded Position + Offset
```

Example: If your yaw servo is 5° off to the right:
- Set `yaw_offset = -5` to compensate

### Step 1: Identify Offsets

1. Command gimbal to center (90°, 90°, 90°)
2. For each axis, measure the actual angle:
   - Use a protractor or angle finder
   - Or use a smartphone level app
3. Calculate offset:
   ```
   Offset = Desired Angle - Actual Angle
   ```

**Example**:
- Commanded: 90°
- Actual: 95°
- Offset = 90 - 95 = -5°

### Step 2: Set Offsets via Web Interface

1. Navigate to **Configuration** tab in web interface
2. Find **Servo Offsets** section
3. Enter calculated offsets:
   - Yaw Offset: `-5` (if yaw is 5° too high)
   - Pitch Offset: `3` (if pitch is 3° too low)
   - Roll Offset: `0` (if roll is perfect)
4. Click **Save Configuration**

### Step 3: Set Offsets via API

```bash
curl -X POST http://[DEVICE_IP]/api/config \
  -H "Content-Type: application/json" \
  -d '{
    "yaw_offset": -5,
    "pitch_offset": 3,
    "roll_offset": 0
  }'
```

### Step 4: Verify Calibration

1. Command gimbal to various positions (0°, 45°, 90°, 135°, 180°)
2. Verify actual position matches commanded position
3. Fine-tune offsets if needed

**Tips**:
- Start with small adjustments (±1° to ±5°)
- Offsets are typically between -10° and +10°
- If you need offsets > ±15°, check mechanical assembly

---

## 3. Flat Reference Setup

The flat reference defines what the system considers "level" or "zero" orientation. This is useful if your gimbal is mounted at an angle.

### When to Use Flat Reference

- Gimbal is permanently mounted at an angle
- You want to define a custom "home" position
- After mechanical changes or repairs

### Method 1: Hardware Button

1. Position gimbal to desired "flat" orientation
2. **Press and hold** the hardware button for **< 3 seconds**
3. Release button (short press)
4. LED will flash to confirm
5. Current position is now saved as flat reference

### Method 2: Web Interface

1. Position gimbal to desired orientation
2. Click **"Set Flat Reference"** button in web interface
3. Confirmation message appears

### Method 3: API

```bash
curl -X POST http://[DEVICE_IP]/api/set-flat-reference
```

### How It Works

The flat reference values are stored in `config.json`:
```json
{
  "flat_ref_yaw": 90.0,
  "flat_ref_pitch": 85.0,
  "flat_ref_roll": 92.0
}
```

When in Auto mode, the system will maintain the gimbal relative to this reference rather than absolute sensor readings.

### Reset Flat Reference

To reset to default (sensor zero):
1. Edit `config.json` and set:
   ```json
   "flat_ref_yaw": -1.0,
   "flat_ref_pitch": -1.0,
   "flat_ref_roll": -1.0
   ```
2. Or perform a factory reset

---

## 4. PID Tuning

PID (Proportional-Integral-Derivative) tuning is required for Auto mode to work smoothly.

### What is PID?

PID is a control algorithm that maintains the gimbal at the target position even when disturbed:
- **Kp (Proportional)**: Responds to current error
- **Ki (Integral)**: Corrects accumulated error over time
- **Kd (Derivative)**: Dampens oscillations

### Default Values

```cpp
Kp = 2.0
Ki = 0.5
Kd = 1.0
```

These work for most setups, but tuning may improve performance.

### Symptoms of Poor Tuning

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| Slow response, lags behind | Kp too low | Increase Kp |
| Oscillates back and forth | Kp too high | Decrease Kp |
| Steady-state error (doesn't reach target) | Ki too low | Increase Ki |
| Slow drift over time | Ki too low | Increase Ki |
| Overshoots and bounces | Kd too low | Increase Kd |
| Jittery, nervous | Kd too high | Decrease Kd |

### Tuning Procedure

#### Step 1: Start with Defaults
```bash
curl -X POST http://[DEVICE_IP]/api/config \
  -H "Content-Type: application/json" \
  -d '{"kp": 2.0, "ki": 0.0, "kd": 0.0}'
```

#### Step 2: Tune Kp (Proportional)

1. Set Ki = 0, Kd = 0
2. Set gimbal to Auto mode
3. Manually move the gimbal (disturb it)
4. Increase Kp until gimbal responds quickly
5. If it oscillates, reduce Kp slightly
6. Ideal: Fast response, no oscillation

**Example progression**:
- Kp = 1.0 → Too slow
- Kp = 2.0 → Good response
- Kp = 3.0 → Starts oscillating
- **Final: Kp = 2.5** (just below oscillation threshold)

#### Step 3: Tune Ki (Integral)

1. Keep Kp at tuned value
2. Slowly increase Ki from 0
3. Check if steady-state error is eliminated
4. Watch for slow oscillations (indicates Ki too high)

**Example progression**:
- Ki = 0.0 → Steady-state error exists
- Ki = 0.3 → Error reduced
- Ki = 0.5 → Error eliminated
- Ki = 0.8 → Slow oscillations
- **Final: Ki = 0.5**

#### Step 4: Tune Kd (Derivative)

1. Keep Kp and Ki at tuned values
2. Increase Kd to reduce overshoot
3. Too much Kd = jittery/nervous behavior

**Example progression**:
- Kd = 0.0 → Overshoots
- Kd = 0.5 → Less overshoot
- Kd = 1.0 → No overshoot
- Kd = 1.5 → Jittery
- **Final: Kd = 1.0**

#### Step 5: Save Configuration

```bash
curl -X POST http://[DEVICE_IP]/api/config \
  -H "Content-Type: application/json" \
  -d '{"kp": 2.5, "ki": 0.5, "kd": 1.0}'
```

### Advanced: Per-Axis Tuning

**Note**: Current firmware uses same PID values for all axes. Future versions may support per-axis tuning.

For now, tune for the most critical axis (usually pitch or roll).

---

## 5. IMU Calibration

The MPU6050 sensor should be factory-calibrated, but you can improve accuracy with manual calibration.

### When to Calibrate IMU

- After physical impacts or drops
- Sensor shows significant drift
- Auto mode doesn't maintain position well

### Calibration Procedure

**Note**: IMU calibration is not currently implemented in firmware. This section describes the theory for future implementation.

#### Gyroscope Calibration (Zero-Rate Offset)

1. Place gimbal on stable, level surface
2. Run calibration routine (records zero-rate offset)
3. Subtract offset from all future readings

#### Accelerometer Calibration

1. Orient sensor in 6 positions (±X, ±Y, ±Z facing up)
2. Record readings in each position
3. Calculate scale and offset for each axis
4. Apply correction to all readings

### Manual IMU Check

1. Monitor serial console
2. Place gimbal perfectly level
3. Check sensor readings:
   - Accel Z should be ~9.8 m/s² (gravity)
   - Accel X and Y should be near 0
   - Gyro X, Y, Z should be near 0 (stationary)

**If readings are far off**:
- Check sensor wiring
- Check I2C address (default 0x68)
- Sensor may be defective

---

## 6. Verification Tests

After calibration, run these tests:

### Test 1: Manual Mode Accuracy

1. Set mode to Manual
2. Command positions: 0°, 45°, 90°, 135°, 180°
3. Verify actual position matches commanded position (±2° tolerance)

### Test 2: Auto Mode Stability

1. Set mode to Auto
2. Set target position (e.g., 90°, 90°, 90°)
3. Manually disturb the gimbal
4. Gimbal should return to target position within 2 seconds
5. No oscillation or overshoot

### Test 3: Smooth Movement

1. Use timed move feature
2. Move from 0° to 180° over 5 seconds
3. Movement should be smooth, no jerking

### Test 4: Self-Test

1. Long-press hardware button (>3 seconds)
2. Gimbal runs self-test routine
3. Each axis should move through full range
4. Check for mechanical binding or issues

---

## 7. Troubleshooting

### Servo Jitters or Shakes

**Possible Causes**:
- Power supply insufficient
- PID gains too high
- Mechanical binding
- Loose connections

**Solutions**:
1. Check power supply (5V 3A minimum)
2. Reduce Kp and Kd
3. Check mechanical assembly for binding
4. Secure all connections

### Gimbal Drifts in Auto Mode

**Possible Causes**:
- Gyro drift (sensor issue)
- Ki too low
- Flat reference not set correctly

**Solutions**:
1. Increase Ki (integral gain)
2. Re-set flat reference
3. Check sensor mounting (must be rigid)

### One Axis Not Moving

**Possible Causes**:
- Servo not connected
- Wrong servo pin
- Servo failed

**Solutions**:
1. Check servo wiring
2. Verify pin numbers in `config.h`
3. Test servo with separate controller
4. Check serial monitor for errors

### Auto Mode Not Working

**Possible Causes**:
- MPU6050 not detected
- Wrong I2C pins
- Sensor not initialized

**Solutions**:
1. Check serial monitor for "MPU6050 Sensor: FAILED"
2. Verify I2C wiring (SDA=GPIO10, SCL=GPIO11)
3. Run I2C scanner to detect sensor address
4. Check sensor power (3.3V)

### Web Interface Shows Wrong Position

**Possible Causes**:
- Servo offsets not applied
- Mechanical slippage
- Servo horn loose

**Solutions**:
1. Re-calibrate servo offsets
2. Tighten servo horns
3. Check mechanical assembly

---

## Calibration Best Practices

1. **Calibrate in order**: Servos → Offsets → Flat Ref → PID
2. **One axis at a time**: Don't change multiple parameters simultaneously
3. **Document values**: Keep a log of your calibration values
4. **Test thoroughly**: Run all verification tests after calibration
5. **Backup config**: Save a backup of `config.json` after successful calibration

---

## Calibration Values Template

Use this template to record your calibration:

```
=== Gimbal Calibration Log ===
Date: _______________
Gimbal ID: _______________

Servo Offsets:
  Yaw:   _____ degrees
  Pitch: _____ degrees
  Roll:  _____ degrees

Flat Reference:
  Yaw:   _____ degrees
  Pitch: _____ degrees
  Roll:  _____ degrees

PID Values:
  Kp: _____
  Ki: _____
  Kd: _____

Notes:
_________________________________
_________________________________
_________________________________
```

---

## Additional Resources

- [GPIO Pin Reference](GPIO_PIN_REFERENCE.md) - Pin assignments and wiring
- [Bench Test Guide](BENCH_TEST_GUIDE.md) - Testing procedures
- [Hardware Setup](HARDWARE_SETUP.md) - Assembly instructions
- [API Documentation](API.md) - API endpoints for calibration
- [Known Issues](../KnownIssues.MD) - Known calibration-related issues

---

## Support

If you encounter issues not covered in this guide:
1. Check [Known Issues](../KnownIssues.MD)
2. Review serial console output
3. Open an issue on GitHub with:
   - Calibration values attempted
   - Observed behavior
   - Serial console logs
   - Hardware configuration

---

**Version History**:
- v1.0 (2026-01-30): Initial calibration guide created
