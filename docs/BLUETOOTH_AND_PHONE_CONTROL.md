# Bluetooth and Phone Gyroscope Control

This document describes the new Bluetooth Low Energy (BLE) and phone gyroscope control features for the ESP32 3-Axis Gimbal system.

## Table of Contents

1. [Bluetooth BLE Overview](#bluetooth-ble-overview)
2. [Phone Gyroscope Control](#phone-gyroscope-control)
3. [BLE API Reference](#ble-api-reference)
4. [Mobile App Development Guide](#mobile-app-development-guide)
5. [Troubleshooting](#troubleshooting)

---

## Bluetooth BLE Overview

The gimbal now supports Bluetooth Low Energy (BLE) for wireless control without requiring WiFi. This is particularly useful for:

- Direct device-to-device communication
- Lower power consumption compared to WiFi
- Mobile app integration
- Offline operation

### Features

- **BLE Device Name**: `ESP32_Gimbal`
- **Connection Status**: Displayed on web interface
- **Real-time Control**: Low-latency gimbal position control
- **Status Updates**: Automatic position and mode notifications

### Security Considerations

**Important**: The current BLE implementation does not include authentication or pairing requirements. This means:

- Any device within Bluetooth range can connect and control the gimbal
- Commands are not encrypted or authenticated
- This is suitable for hobbyist/educational use in controlled environments

**For production or security-sensitive applications**, consider:

1. Implementing BLE pairing/bonding requirements
2. Adding application-level authentication tokens
3. Using encrypted characteristics
4. Implementing command validation and rate limiting

See the "Future Enhancements" section for planned security features.

---

## Connecting via Bluetooth

### User Guide (How to connect)

**Important**: This system uses standard Bluetooth Low Energy (BLE) GATT. It does **not** support "Fast Pair" or "Find My" style automatic popups like AirTags or AirPods. You must use a compatible app to connect.

#### 1. Using a Generic BLE Scanner App (Recommended for Testing)

To test the connection or control the gimbal manually without building a custom app, use a free BLE scanner app:
- **iOS**: [LightBlue](https://apps.apple.com/us/app/lightblue/id557428110) or [nRF Connect](https://apps.apple.com/us/app/nrf-connect-for-mobile/id1338636408)
- **Android**: [nRF Connect](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) or [LightBlue](https://play.google.com/store/apps/details?id=com.punchthrough.lightblueexplorer)

**Steps:**
1. Open the app and scan for devices.
2. Find **`ESP32_Gimbal`** in the list and tap **Connect**.
3. **No pairing code is required.** If prompted, just select "Pair" (some phones force this) or cancel; the connection is open.
4. Once connected:
   - Browse the "Services".
   - Find the custom service `4fafc201-1fb5-459e-8fcc-c5c9c331914b`.
   - You can write to the characteristics to control the gimbal (see API Reference below).

#### 2. Using the WiFi Web Interface?
**Note**: The Web Interface (accessed via `http://192.168.4.1`) uses **WiFi**, not Bluetooth.
- The "Bluetooth" status indicator in the Web Interface simply tells you if *another* device is currently controlling the gimbal via BLE.
- You cannot establish the BLE connection *from* the Web Interface because browsers do not support Web Bluetooth over insecure HTTP connections (only HTTPS or localhost).

### FAQ

**Q: Do I have to pair?**
**A: No.** The device uses open BLE advertising. No bonding or PIN code is required.

**Q: Does it work like AirTags?**
**A: No.** AirTags use proprietary Apple protocols for background discovery and popups. This device uses standard BLE. It will not automatically pop up on your phone unless you have an app installed that is scanning for it.

---

## Phone Gyroscope Control

The web interface now supports using your phone's built-in gyroscope and accelerometer to control the gimbal tilt in real-time. This creates an intuitive control method where tilting your phone directly controls the gimbal orientation.

### Features

- **Real-time Tilt Control**: Tilt your phone to control gimbal angles
- **Visual Feedback**: See orientation data on screen
- **Easy Toggle**: Enable/disable with a single button
- **iOS & Android Support**: Works on modern mobile browsers
- **Permission Handling**: Automatically requests necessary permissions

### How to Use Phone Gyroscope Control

#### Step 1: Access the Web Interface on Your Phone

1. Connect your phone to the gimbal's WiFi network or hotspot
2. Open a web browser (Safari, Chrome, Firefox)
3. Navigate to the gimbal's IP address (e.g., `http://192.168.4.1`)

#### Step 2: Enable Phone Gyroscope

1. Scroll to the **Phone Gyroscope Control** section
2. Tap the **Enable** button
3. If prompted (iOS 13+), grant permission to access motion sensors
4. The status will show "Active: Receiving motion data..."

#### Step 3: Control the Gimbal

- **Yaw (Left/Right)**: Rotate your phone horizontally
- **Pitch (Forward/Back)**: Tilt your phone forward or backward
- **Roll (Side/Side)**: Tilt your phone left or right

The gimbal will respond in real-time to your phone's orientation.

#### Step 4: Disable When Done

Tap the **Disable** button to stop phone gyroscope control.

### Orientation Mapping

The phone's orientation sensors provide three values:

- **Alpha (Z-axis)**: Compass heading (0° to 360°) → Maps to Yaw (0° to 180°)
- **Beta (X-axis)**: Front-back tilt (-180° to 180°) → Maps to Pitch (0° to 180°)
- **Gamma (Y-axis)**: Left-right tilt (-90° to 90°) → Maps to Roll (0° to 180°)

### Browser Compatibility

| Browser | Platform | Support |
|---------|----------|---------|
| Safari | iOS 13+ | ✅ Yes (requires permission) |
| Chrome | Android | ✅ Yes |
| Firefox | Android | ✅ Yes |
| Chrome | Desktop | ⚠️ Limited (no motion sensors) |
| Safari | macOS | ⚠️ Limited (no motion sensors) |

**Note**: Motion sensors are typically only available on mobile devices (phones and tablets).

---

## BLE API Reference

### Service UUID

```
4fafc201-1fb5-459e-8fcc-c5c9c331914b
```

### Characteristics

#### 1. Position Control (Write)

**UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`

**Format**: 12 bytes (3 floats)

```
Byte 0-3:   Yaw (float)
Byte 4-7:   Pitch (float)
Byte 8-11:  Roll (float)
```

**Example** (JavaScript):

```javascript
const yaw = 90.0;
const pitch = 90.0;
const roll = 90.0;

const buffer = new ArrayBuffer(12);
const view = new DataView(buffer);
view.setFloat32(0, yaw, true);    // Little endian
view.setFloat32(4, pitch, true);
view.setFloat32(8, roll, true);

await positionCharacteristic.writeValue(buffer);
```

#### 2. Mode Control (Write)

**UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a9`

**Format**: 1 byte

```
0 = Manual Mode
1 = Auto Mode
```

**Example** (JavaScript):

```javascript
const mode = 0; // Manual
await modeCharacteristic.writeValue(new Uint8Array([mode]));
```

#### 3. Status (Read/Notify)

**UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26aa`

**Format**: 13 bytes

```
Byte 0:     Mode (uint8)
Byte 1-4:   Current Yaw (float)
Byte 5-8:   Current Pitch (float)
Byte 9-12:  Current Roll (float)
```

**Example** (JavaScript):

```javascript
// Read status
const value = await statusCharacteristic.readValue();
const mode = value.getUint8(0);
const yaw = value.getFloat32(1, true);
const pitch = value.getFloat32(5, true);
const roll = value.getFloat32(9, true);

// Listen for notifications
statusCharacteristic.addEventListener('characteristicvaluechanged', (event) => {
    const value = event.target.value;
    const mode = value.getUint8(0);
    const yaw = value.getFloat32(1, true);
    const pitch = value.getFloat32(5, true);
    const roll = value.getFloat32(9, true);
    
    console.log(`Mode: ${mode}, Yaw: ${yaw}, Pitch: ${pitch}, Roll: ${roll}`);
});

await statusCharacteristic.startNotifications();
```

---

## Mobile App Development Guide

### Web Bluetooth API (JavaScript)

```javascript
async function connectToGimbal() {
    try {
        // Request device
        const device = await navigator.bluetooth.requestDevice({
            filters: [{ name: 'ESP32_Gimbal' }],
            optionalServices: ['4fafc201-1fb5-459e-8fcc-c5c9c331914b']
        });
        
        // Connect to GATT server
        const server = await device.gatt.connect();
        
        // Get service
        const service = await server.getPrimaryService('4fafc201-1fb5-459e-8fcc-c5c9c331914b');
        
        // Get characteristics
        const positionChar = await service.getCharacteristic('beb5483e-36e1-4688-b7f5-ea07361b26a8');
        const modeChar = await service.getCharacteristic('beb5483e-36e1-4688-b7f5-ea07361b26a9');
        const statusChar = await service.getCharacteristic('beb5483e-36e1-4688-b7f5-ea07361b26aa');
        
        // Set position
        const buffer = new ArrayBuffer(12);
        const view = new DataView(buffer);
        view.setFloat32(0, 90.0, true);  // Yaw
        view.setFloat32(4, 90.0, true);  // Pitch
        view.setFloat32(8, 90.0, true);  // Roll
        await positionChar.writeValue(buffer);
        
        // Set mode to Manual
        await modeChar.writeValue(new Uint8Array([0]));
        
        // Listen for status updates
        statusChar.addEventListener('characteristicvaluechanged', handleStatusUpdate);
        await statusChar.startNotifications();
        
        console.log('Connected to gimbal!');
        
    } catch (error) {
        console.error('Connection failed:', error);
    }
}

function handleStatusUpdate(event) {
    const value = event.target.value;
    const mode = value.getUint8(0);
    const yaw = value.getFloat32(1, true);
    const pitch = value.getFloat32(5, true);
    const roll = value.getFloat32(9, true);
    
    console.log(`Status - Mode: ${mode}, Position: [${yaw}, ${pitch}, ${roll}]`);
}
```

### React Native (using react-native-ble-plx)

```javascript
import { BleManager } from 'react-native-ble-plx';

const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const POSITION_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const MODE_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a9';
const STATUS_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26aa';

const manager = new BleManager();

async function connectToGimbal() {
    // Scan for device
    const device = await manager.scanAndConnectDevice('ESP32_Gimbal');
    
    // Discover services and characteristics
    await device.discoverAllServicesAndCharacteristics();
    
    // Set position
    const yaw = 90.0;
    const pitch = 90.0;
    const roll = 90.0;
    
    const buffer = Buffer.allocUnsafe(12);
    buffer.writeFloatLE(yaw, 0);
    buffer.writeFloatLE(pitch, 4);
    buffer.writeFloatLE(roll, 8);
    
    await device.writeCharacteristicWithResponseForService(
        SERVICE_UUID,
        POSITION_CHAR_UUID,
        buffer.toString('base64')
    );
    
    // Monitor status
    device.monitorCharacteristicForService(
        SERVICE_UUID,
        STATUS_CHAR_UUID,
        (error, characteristic) => {
            if (error) {
                console.error(error);
                return;
            }
            
            const data = Buffer.from(characteristic.value, 'base64');
            const mode = data.readUInt8(0);
            const yaw = data.readFloatLE(1);
            const pitch = data.readFloatLE(5);
            const roll = data.readFloatLE(9);
            
            console.log(`Mode: ${mode}, Yaw: ${yaw}, Pitch: ${pitch}, Roll: ${roll}`);
        }
    );
}
```

### Flutter (using flutter_blue_plus)

```dart
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

final serviceUuid = Guid('4fafc201-1fb5-459e-8fcc-c5c9c331914b');
final positionCharUuid = Guid('beb5483e-36e1-4688-b7f5-ea07361b26a8');
final modeCharUuid = Guid('beb5483e-36e1-4688-b7f5-ea07361b26a9');
final statusCharUuid = Guid('beb5483e-36e1-4688-b7f5-ea07361b26aa');

Future<void> connectToGimbal() async {
  // Scan for device
  FlutterBluePlus.startScan(timeout: Duration(seconds: 4));
  
  var subscription = FlutterBluePlus.scanResults.listen((results) {
    for (ScanResult r in results) {
      if (r.device.name == 'ESP32_Gimbal') {
        // Connect
        r.device.connect();
      }
    }
  });
  
  await FlutterBluePlus.stopScan();
  
  // Discover services
  List<BluetoothService> services = await device.discoverServices();
  
  // Find our service
  var service = services.firstWhere((s) => s.uuid == serviceUuid);
  
  // Get characteristics
  var positionChar = service.characteristics.firstWhere((c) => c.uuid == positionCharUuid);
  var statusChar = service.characteristics.firstWhere((c) => c.uuid == statusCharUuid);
  
  // Set position
  double yaw = 90.0;
  double pitch = 90.0;
  double roll = 90.0;
  
  var bytes = ByteData(12)
    ..setFloat32(0, yaw, Endian.little)
    ..setFloat32(4, pitch, Endian.little)
    ..setFloat32(8, roll, Endian.little);
  
  await positionChar.write(bytes.buffer.asUint8List());
  
  // Subscribe to status updates
  await statusChar.setNotifyValue(true);
  statusChar.value.listen((value) {
    var data = ByteData.sublistView(Uint8List.fromList(value));
    int mode = data.getUint8(0);
    double yaw = data.getFloat32(1, Endian.little);
    double pitch = data.getFloat32(5, Endian.little);
    double roll = data.getFloat32(9, Endian.little);
    
    print('Mode: $mode, Yaw: $yaw, Pitch: $pitch, Roll: $roll');
  });
}
```

---

## Troubleshooting

### Bluetooth Connection Issues

**Problem**: Cannot find `ESP32_Gimbal` device

**Solutions**:
1. Ensure the ESP32 is powered on
2. Check that Bluetooth is enabled on your phone/device
3. Move closer to the ESP32 (within ~10 meters)
4. Restart the ESP32
5. Restart Bluetooth on your mobile device

**Problem**: Connection drops frequently

**Solutions**:
1. Reduce distance between devices
2. Minimize obstacles (walls, metal objects)
3. Check for interference from other Bluetooth devices
4. Ensure ESP32 has stable power supply

### Phone Gyroscope Issues

**Problem**: "Device orientation not supported" error

**Solutions**:
1. Ensure you're using a mobile device (phones/tablets have motion sensors)
2. Use a modern browser (Chrome, Safari, Firefox)
3. Check that your device has a gyroscope (most modern smartphones do)

**Problem**: "Permission denied" on iOS

**Solutions**:
1. iOS 13+ requires explicit permission for motion sensors
2. Tap the "Enable" button which triggers the permission request
3. Accept the permission dialog when it appears
4. If denied, go to Safari Settings → Motion & Orientation Access

**Problem**: Gimbal movement is jittery or erratic

**Solutions**:
1. Hold your phone steadily
2. Calibrate your phone's gyroscope (in phone settings)
3. The mapping might need adjustment for your use case
4. Try manual control mode for more precise movements

**Problem**: Phone gyroscope not working in desktop browser

**Solution**: Motion sensors are only available on mobile devices. Desktop computers typically don't have gyroscopes or accelerometers.

### Web Interface Issues

**Problem**: Bluetooth status shows "Disconnected" even when connected

**Solutions**:
1. Refresh the web page
2. The status updates every 100ms via WebSocket
3. Check WebSocket connection indicator (green dot in top right)
4. Ensure you're connected to the gimbal's network

**Problem**: Phone gyroscope control is slow/laggy

**Solutions**:
1. Ensure good WiFi/network connection
2. Close other apps/tabs that might be using resources
3. The update rate is limited by WebSocket communication
4. Try reducing distance to the ESP32

---

## Advanced Usage

### Combining Phone Gyroscope with Bluetooth

For the lowest latency control, consider developing a mobile app that:
1. Reads phone gyroscope data locally
2. Sends commands via Bluetooth BLE directly
3. Bypasses the web interface entirely

This approach can achieve <50ms latency for real-time control.

### Custom Mapping

The current phone orientation mapping can be customized by modifying the WebSocket command handler in `WebManager.cpp`:

```cpp
// Current mapping in WebManager.cpp
float yaw = alpha / 2.0;     // Map 0-360 to 0-180
float pitch = ((beta + 180.0) / 360.0) * 180.0;
float roll = ((gamma + 90.0) / 180.0) * 180.0;
```

Adjust these formulas to match your desired control scheme.

---

## Future Enhancements

Planned features for future releases:

- [ ] Bluetooth Classic support (in addition to BLE)
- [ ] Sensitivity adjustment for phone gyroscope
- [ ] Smoothing/filtering options for phone control
- [ ] Save/load phone calibration profiles
- [ ] Haptic feedback for mobile devices
- [ ] Native mobile apps for iOS and Android
- [ ] Bluetooth pairing security

---

## Support

For issues or questions:
- GitHub Issues: [Repository URL]
- Documentation: See `/docs` folder
- Community: [Discord/Forum link]

---

**Version**: 1.3.0  
**Last Updated**: January 2024
