# API Documentation

Complete API reference for the ESP32 3-Axis Gimbal Control System.

## Base URLs

- **ESP32 Direct**: `http://[ESP32_IP_ADDRESS]`
- **ESP32 Hotspot**: `http://192.168.4.1`
- **FastAPI Backend**: `http://localhost:8000`

## Authentication

Currently, authentication is not required. For production deployments, consider implementing:
- JWT tokens
- API keys
- OAuth 2.0

## REST API Endpoints

### Status & Health

#### GET /api/status
Get current gimbal status and sensor data.

**Response:**
```json
{
  "mode": 0,
  "wifi_connected": true,
  "hotspot_mode": false,
  "position": {
    "yaw": 90.0,
    "pitch": 90.0,
    "roll": 90.0
  }
}
```

#### GET /api/health (FastAPI only)
Health check endpoint.

**Response:**
```json
{
  "status": "healthy",
  "timestamp": "2024-01-01T12:00:00"
}
```

### Mode Control

#### POST /api/mode
Set operation mode.

**Query Parameters:**
- `mode` (integer): 0 for Manual, 1 for Auto

**Example:**
```bash
curl -X POST "http://192.168.4.1/api/mode?mode=0"
```

**Response:**
```json
{
  "status": "ok",
  "mode": 0
}
```

### Position Control

#### POST /api/position
Set gimbal position (Manual mode only).

**Request Body:**
```json
{
  "yaw": 120.0,
  "pitch": 90.0,
  "roll": 60.0
}
```

**Example:**
```bash
curl -X POST "http://192.168.4.1/api/position" \
  -H "Content-Type: application/json" \
  -d '{"yaw": 120, "pitch": 90, "roll": 60}'
```

**Response:**
```json
{
  "status": "ok",
  "position": {
    "yaw": 120.0,
    "pitch": 90.0,
    "roll": 60.0
  }
}
```

**Constraints:**
- Yaw: 0-180 degrees
- Pitch: 0-180 degrees
- Roll: 0-180 degrees

#### POST /api/center
Center the gimbal to neutral position (90°, 90°, 90°).

**Example:**
```bash
curl -X POST "http://192.168.4.1/api/center"
```

**Response:**
```json
{
  "status": "ok",
  "position": {
    "yaw": 90.0,
    "pitch": 90.0,
    "roll": 90.0
  }
}
```

### Auto Mode

#### POST /api/auto-target
Set target position for Auto mode stabilization.

**Request Body:**
```json
{
  "yaw": 90.0,
  "pitch": 90.0,
  "roll": 90.0
}
```

**Example:**
```bash
curl -X POST "http://192.168.4.1/api/auto-target" \
  -H "Content-Type: application/json" \
  -d '{"yaw": 90, "pitch": 90, "roll": 90}'
```

**Response:**
```json
{
  "status": "ok",
  "target": {
    "yaw": 90.0,
    "pitch": 90.0,
    "roll": 90.0
  }
}
```

### Timed Moves

#### POST /api/timed-move
Start a timed movement from current position to target position.

**Request Body:**
```json
{
  "duration": 5000,
  "end_position": {
    "yaw": 120.0,
    "pitch": 100.0,
    "roll": 80.0
  }
}
```

**Parameters:**
- `duration` (integer): Duration in milliseconds
- `end_position` (object): Target position

**Example:**
```bash
curl -X POST "http://192.168.4.1/api/timed-move" \
  -H "Content-Type: application/json" \
  -d '{
    "duration": 5000,
    "end_position": {"yaw": 120, "pitch": 100, "roll": 80}
  }'
```

**Response:**
```json
{
  "status": "ok",
  "move": {
    "duration": 5000,
    "end_position": {
      "yaw": 120.0,
      "pitch": 100.0,
      "roll": 80.0
    }
  }
}
```

### Preset Moves (FastAPI Backend)

#### GET /api/presets
List all saved preset moves.

**Response:**
```json
{
  "presets": [
    {
      "name": "sweep",
      "description": "Sweep from left to right",
      "steps": [
        {
          "position": {"yaw": 0, "pitch": 90, "roll": 90},
          "duration": 2000,
          "delay": 0
        },
        {
          "position": {"yaw": 180, "pitch": 90, "roll": 90},
          "duration": 5000,
          "delay": 500
        }
      ]
    }
  ]
}
```

#### POST /api/presets
Create a new preset move.

**Request Body:**
```json
{
  "name": "sweep",
  "description": "Sweep from left to right",
  "steps": [
    {
      "position": {"yaw": 0, "pitch": 90, "roll": 90},
      "duration": 2000,
      "delay": 0
    },
    {
      "position": {"yaw": 180, "pitch": 90, "roll": 90},
      "duration": 5000,
      "delay": 500
    }
  ]
}
```

**Response:**
```json
{
  "status": "ok",
  "preset": { /* preset object */ }
}
```

#### POST /api/presets/{preset_name}/execute
Execute a saved preset move.

**Example:**
```bash
curl -X POST "http://localhost:8000/api/presets/sweep/execute"
```

**Response:**
```json
{
  "status": "ok",
  "preset": { /* preset object */ }
}
```

#### DELETE /api/presets/{preset_name}
Delete a preset move.

**Example:**
```bash
curl -X DELETE "http://localhost:8000/api/presets/sweep"
```

**Response:**
```json
{
  "status": "ok"
}
```

## WebSocket API

### Connection

**Endpoint:** `/ws`

**URL:** `ws://[ESP32_IP]/ws` or `ws://192.168.4.1/ws`

### Messages from ESP32

The ESP32 sends periodic updates with sensor and position data:

```json
{
  "mode": 0,
  "position": {
    "yaw": 90.5,
    "pitch": 89.8,
    "roll": 90.2
  },
  "sensors": {
    "accel": {
      "x": 0.05,
      "y": 0.02,
      "z": 9.81
    },
    "gyro": {
      "x": 0.001,
      "y": -0.002,
      "z": 0.000
    }
  }
}
```

### Messages to ESP32

#### Set Mode
```json
{
  "cmd": "setMode",
  "mode": 1
}
```

#### Set Position (Manual Mode)
```json
{
  "cmd": "setPosition",
  "yaw": 120,
  "pitch": 90,
  "roll": 60
}
```

#### Set Auto Target
```json
{
  "cmd": "setAutoTarget",
  "yaw": 90,
  "pitch": 90,
  "roll": 90
}
```

#### Start Timed Move
```json
{
  "cmd": "startTimedMove",
  "duration": 5000,
  "endYaw": 120,
  "endPitch": 100,
  "endRoll": 80
}
```

## Code Examples

### JavaScript/Web

```javascript
// Connect to WebSocket
const ws = new WebSocket('ws://192.168.4.1/ws');

ws.onopen = () => {
  console.log('Connected to gimbal');
};

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  console.log('Position:', data.position);
  console.log('Sensors:', data.sensors);
};

// Set manual position
function setPosition(yaw, pitch, roll) {
  ws.send(JSON.stringify({
    cmd: 'setPosition',
    yaw: yaw,
    pitch: pitch,
    roll: roll
  }));
}

// Change mode
function setMode(mode) {
  ws.send(JSON.stringify({
    cmd: 'setMode',
    mode: mode  // 0 = Manual, 1 = Auto
  }));
}
```

### Python

```python
import requests
import json
from websockets import connect
import asyncio

BASE_URL = "http://192.168.4.1"

# Set position
def set_position(yaw, pitch, roll):
    response = requests.post(
        f"{BASE_URL}/api/position",
        json={"yaw": yaw, "pitch": pitch, "roll": roll}
    )
    return response.json()

# Set mode
def set_mode(mode):
    response = requests.post(f"{BASE_URL}/api/mode?mode={mode}")
    return response.json()

# WebSocket client
async def listen_to_gimbal():
    uri = "ws://192.168.4.1/ws"
    async with connect(uri) as websocket:
        while True:
            message = await websocket.recv()
            data = json.loads(message)
            print(f"Position: {data['position']}")
            print(f"Sensors: {data['sensors']}")

# Run WebSocket client
asyncio.run(listen_to_gimbal())
```

### cURL Examples

```bash
# Get status
curl http://192.168.4.1/api/status

# Set manual mode
curl -X POST "http://192.168.4.1/api/mode?mode=0"

# Set position
curl -X POST "http://192.168.4.1/api/position" \
  -H "Content-Type: application/json" \
  -d '{"yaw": 120, "pitch": 90, "roll": 60}'

# Center gimbal
curl -X POST "http://192.168.4.1/api/center"

# Start timed move
curl -X POST "http://192.168.4.1/api/timed-move" \
  -H "Content-Type: application/json" \
  -d '{
    "duration": 5000,
    "end_position": {"yaw": 120, "pitch": 100, "roll": 80}
  }'
```

### Node.js

```javascript
const WebSocket = require('ws');
const axios = require('axios');

const BASE_URL = 'http://192.168.4.1';
const WS_URL = 'ws://192.168.4.1/ws';

// REST API calls
async function setPosition(yaw, pitch, roll) {
  const response = await axios.post(`${BASE_URL}/api/position`, {
    yaw, pitch, roll
  });
  return response.data;
}

async function setMode(mode) {
  const response = await axios.post(`${BASE_URL}/api/mode?mode=${mode}`);
  return response.data;
}

// WebSocket connection
const ws = new WebSocket(WS_URL);

ws.on('open', () => {
  console.log('Connected to gimbal');
});

ws.on('message', (data) => {
  const message = JSON.parse(data);
  console.log('Position:', message.position);
  console.log('Sensors:', message.sensors);
});

ws.on('error', (error) => {
  console.error('WebSocket error:', error);
});
```

### React/React Native

```jsx
import React, { useState, useEffect } from 'react';

function GimbalController() {
  const [ws, setWs] = useState(null);
  const [position, setPosition] = useState({ yaw: 90, pitch: 90, roll: 90 });
  const [sensors, setSensors] = useState(null);

  useEffect(() => {
    const websocket = new WebSocket('ws://192.168.4.1/ws');
    
    websocket.onopen = () => {
      console.log('Connected');
    };
    
    websocket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      setPosition(data.position);
      setSensors(data.sensors);
    };
    
    setWs(websocket);
    
    return () => websocket.close();
  }, []);

  const updatePosition = (axis, value) => {
    const newPosition = { ...position, [axis]: value };
    setPosition(newPosition);
    
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({
        cmd: 'setPosition',
        ...newPosition
      }));
    }
  };

  return (
    <div>
      <h2>Gimbal Control</h2>
      <div>
        <label>Yaw: {position.yaw}</label>
        <input 
          type="range" 
          min="0" 
          max="180" 
          value={position.yaw}
          onChange={(e) => updatePosition('yaw', e.target.value)}
        />
      </div>
      {/* Similar controls for pitch and roll */}
    </div>
  );
}
```

## Error Responses

### 400 Bad Request
```json
{
  "detail": "Invalid mode. Use 0 for Manual or 1 for Auto"
}
```

### 404 Not Found
```json
{
  "detail": "Preset not found"
}
```

### 500 Internal Server Error
```json
{
  "detail": "Internal server error"
}
```

## Rate Limiting

Currently no rate limiting is implemented. For production:
- Implement rate limiting middleware
- Set appropriate limits (e.g., 100 requests/minute)
- Use API keys for identification

## API Versioning

Current version: v1.0.0

Future versions will be accessible via:
- URL versioning: `/api/v2/...`
- Header versioning: `Accept: application/vnd.gimbal.v2+json`

## Interactive API Documentation

When using FastAPI backend:
- Swagger UI: `http://localhost:8000/docs`
- ReDoc: `http://localhost:8000/redoc`
- OpenAPI JSON: `http://localhost:8000/openapi.json`
