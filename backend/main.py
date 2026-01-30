from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from pydantic import BaseModel
from typing import Optional, List
import orjson
import asyncio
from datetime import datetime

app = FastAPI(title="3-Axis Gimbal API", version="1.0.0")

# CORS middleware
# ⚠️ SECURITY ISSUE: See KnownIssues.MD #ISSUE-001
# TODO: Restrict CORS origins before production deployment
# Current configuration allows ANY origin to make authenticated requests (SECURITY RISK)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # ⚠️ CHANGE THIS: Use specific origins in production
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Data Models
class GimbalPosition(BaseModel):
    yaw: float
    pitch: float
    roll: float

class GimbalConfig(BaseModel):
    mode: int  # 0 = Manual, 1 = Auto
    wifi_ssid: Optional[str] = None
    wifi_password: Optional[str] = None
    hotspot_ssid: Optional[str] = "Gimbal_AP"
    hotspot_password: Optional[str] = "gimbal123"

class TimedMove(BaseModel):
    duration: int  # milliseconds
    end_position: GimbalPosition

class PresetMove(BaseModel):
    name: str
    positions: List[dict]  # List of {position, duration, delay}

# In-memory storage (should be replaced with database in production)
gimbal_state = {
    "mode": 0,
    "position": {"yaw": 90, "pitch": 90, "roll": 90},
    "auto_target": {"yaw": 90, "pitch": 90, "roll": 90},
    "sensors": {
        "accel": {"x": 0, "y": 0, "z": 0},
        "gyro": {"x": 0, "y": 0, "z": 0}
    },
    "connected": False
}

preset_moves = {}

# WebSocket connection manager
class ConnectionManager:
    def __init__(self):
        self.active_connections: List[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        self.active_connections.remove(websocket)

    async def broadcast(self, message: str):
        # ⚠️ RELIABILITY ISSUE: See KnownIssues.MD #ISSUE-008
        # TODO: Properly handle dead connections and remove them
        for connection in self.active_connections:
            try:
                await connection.send_text(message)
            except:
                pass  # Silent failure - dead connections accumulate

manager = ConnectionManager()

# Root endpoint
@app.get("/")
async def root():
    return {
        "message": "3-Axis Gimbal API",
        "version": "1.0.0",
        "docs": "/docs"
    }

# Health check
@app.get("/api/health")
async def health_check():
    return {
        "status": "healthy",
        "timestamp": datetime.now().isoformat()
    }

# Get gimbal status
# ⚠️ SECURITY ISSUE: See KnownIssues.MD #ISSUE-002
# TODO: Add authentication before production deployment
@app.get("/api/status")
async def get_status():
    return gimbal_state

# Set operation mode
@app.post("/api/mode")
async def set_mode(mode: int):
    if mode not in [0, 1]:
        raise HTTPException(status_code=400, detail="Invalid mode. Use 0 for Manual or 1 for Auto")
    
    gimbal_state["mode"] = mode
    await manager.broadcast(orjson.dumps({"cmd": "mode_changed", "mode": mode}).decode("utf-8"))
    return {"status": "ok", "mode": mode}

# Set manual position
# ⚠️ SECURITY ISSUE: See KnownIssues.MD #ISSUE-002 (No authentication)
# ⚠️ SECURITY ISSUE: See KnownIssues.MD #ISSUE-006 (No input validation)
# ⚠️ SECURITY ISSUE: See KnownIssues.MD #ISSUE-007 (No rate limiting)
@app.post("/api/position")
async def set_position(position: GimbalPosition):
    # TODO: Add authentication check
    # TODO: Add input range validation (0-180 degrees)
    # TODO: Add rate limiting
    if gimbal_state["mode"] != 0:
        raise HTTPException(status_code=400, detail="Gimbal must be in manual mode")
    
    gimbal_state["position"] = position.dict()
    await manager.broadcast(orjson.dumps({"cmd": "position_update", "position": position.dict()}).decode("utf-8"))
    return {"status": "ok", "position": position.dict()}

# Set auto mode target
@app.post("/api/auto-target")
async def set_auto_target(position: GimbalPosition):
    gimbal_state["auto_target"] = position.dict()
    await manager.broadcast(orjson.dumps({"cmd": "auto_target_update", "target": position.dict()}).decode("utf-8"))
    return {"status": "ok", "target": position.dict()}

# Start timed move
@app.post("/api/timed-move")
async def start_timed_move(move: TimedMove):
    command = {
        "cmd": "timed_move",
        "duration": move.duration,
        "end_position": move.end_position.dict()
    }
    await manager.broadcast(orjson.dumps(command).decode("utf-8"))
    return {"status": "ok", "move": move.dict()}

# Get all preset moves
@app.get("/api/presets")
async def get_presets():
    return {"presets": list(preset_moves.values())}

# Create preset move
@app.post("/api/presets")
async def create_preset(preset: PresetMove):
    preset_moves[preset.name] = preset.dict()
    return {"status": "ok", "preset": preset.dict()}

# Execute preset move
@app.post("/api/presets/{preset_name}/execute")
async def execute_preset(preset_name: str):
    preset = preset_moves.get(preset_name)
    if not preset:
        raise HTTPException(status_code=404, detail="Preset not found")
    
    await manager.broadcast(orjson.dumps({"cmd": "execute_preset", "preset": preset}).decode("utf-8"))
    return {"status": "ok", "preset": preset}

# Delete preset
@app.delete("/api/presets/{preset_name}")
async def delete_preset(preset_name: str):
    if preset_name in preset_moves:
        del preset_moves[preset_name]
    return {"status": "ok"}

# Center gimbal
@app.post("/api/center")
async def center_gimbal():
    center_position = {"yaw": 90, "pitch": 90, "roll": 90}
    gimbal_state["position"] = center_position
    await manager.broadcast(orjson.dumps({"cmd": "center"}).decode("utf-8"))
    return {"status": "ok", "position": center_position}

# WebSocket endpoint for real-time communication
# ⚠️ SECURITY ISSUE: See KnownIssues.MD #ISSUE-005
# TODO: Implement WebSocket authentication before production
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    try:
        # Send current state on connection
        await websocket.send_text(orjson.dumps(gimbal_state).decode("utf-8"))
        
        while True:
            data = await websocket.receive_text()
            message = orjson.loads(data)
            
            # Handle incoming messages from ESP32 or clients
            if message.get("type") == "sensor_update":
                gimbal_state["sensors"] = message.get("sensors", {})
                gimbal_state["position"] = message.get("position", gimbal_state["position"])
                await manager.broadcast(data)
            
            elif message.get("type") == "status_update":
                gimbal_state.update(message.get("state", {}))
                await manager.broadcast(data)
            
    except WebSocketDisconnect:
        manager.disconnect(websocket)

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
