import json
import timeit
import sys

# Mock data
gimbal_state = {
    "mode": 0,
    "position": {"yaw": 90, "pitch": 90, "roll": 90},
    "auto_target": {"yaw": 90, "pitch": 90, "roll": 90},
    "sensors": {
        "accel": {"x": 0.12, "y": -0.05, "z": 9.81},
        "gyro": {"x": 0.01, "y": 0.02, "z": -0.01}
    },
    "connected": False
}

ITERATIONS = 100000

def bench_json():
    json.dumps(gimbal_state)

def bench_ujson():
    import ujson
    ujson.dumps(gimbal_state)

def bench_orjson():
    import orjson
    orjson.dumps(gimbal_state)

def bench_orjson_str():
    import orjson
    orjson.dumps(gimbal_state).decode('utf-8')

print(f"Benchmarking with {ITERATIONS} iterations...")

# Standard json
t_json = timeit.timeit(bench_json, number=ITERATIONS)
print(f"json: {t_json:.4f} s")

# ujson
try:
    import ujson
    t_ujson = timeit.timeit(bench_ujson, number=ITERATIONS)
    print(f"ujson: {t_ujson:.4f} s (Speedup: {t_json/t_ujson:.2f}x)")
except ImportError:
    print("ujson not installed")

# orjson
try:
    import orjson
    t_orjson = timeit.timeit(bench_orjson, number=ITERATIONS)
    print(f"orjson (bytes): {t_orjson:.4f} s (Speedup: {t_json/t_orjson:.2f}x)")

    t_orjson_str = timeit.timeit(bench_orjson_str, number=ITERATIONS)
    print(f"orjson (str): {t_orjson_str:.4f} s (Speedup: {t_json/t_orjson_str:.2f}x)")
except ImportError:
    print("orjson not installed")
