import os
import json
import sys
from SCons.Script import Import

Import("env")

def verify_web_assets(source, target, env):
    print("Verifying web assets...")
    project_dir = env.get("PROJECT_DIR")
    data_dir = os.path.join(project_dir, "data")
    required_files = ["index.html", "config.json", "favicon.svg"]

    missing_files = []

    if not os.path.exists(data_dir):
        print(f"Error: Data directory not found at {data_dir}")
        env.Exit(1)

    for f in required_files:
        if not os.path.exists(os.path.join(data_dir, f)):
            missing_files.append(f)

    if missing_files:
        print(f"Error: Missing web assets in {data_dir}: {missing_files}")
        env.Exit(1)

    # Validate JSON
    config_path = os.path.join(data_dir, "config.json")
    try:
        with open(config_path, 'r') as f:
            json.load(f)
        print(f"✓ {config_path} is valid JSON")
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in {config_path}: {e}")
        env.Exit(1)

    print("✓ Web assets verification passed.")

# Hook into build/upload processes
# Run before building filesystem image
env.AddPreAction("buildfs", verify_web_assets)
env.AddPreAction("uploadfs", verify_web_assets)
