from pydantic import BaseModel
from typing import Optional, List

class GimbalPosition(BaseModel):
    yaw: float
    pitch: float
    roll: float

class SensorData(BaseModel):
    accel: dict
    gyro: dict

class GimbalState(BaseModel):
    mode: int
    position: GimbalPosition
    auto_target: GimbalPosition
    sensors: SensorData
    connected: bool

class Configuration(BaseModel):
    wifi_ssid: Optional[str] = None
    wifi_password: Optional[str] = None
    hotspot_ssid: str = "Gimbal_AP"
    hotspot_password: str = "gimbal123"
    servo_pins: dict = {"yaw": 13, "pitch": 12, "roll": 14}
    pid_params: dict = {"kp": 2.0, "ki": 0.5, "kd": 1.0}

class TimedMoveStep(BaseModel):
    position: GimbalPosition
    duration: int  # milliseconds
    delay: int = 0  # delay before starting this step

class PresetMove(BaseModel):
    name: str
    description: Optional[str] = None
    steps: List[TimedMoveStep]
