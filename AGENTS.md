# AGENTS.md

## Codebase Structure
This codebase follows Domain-Driven Design (DDD) and Separation of Concerns (SOC) principles.

### ESP32 Firmware (`esp32_firmware/`)
The firmware is refactored from a monolith into modular components:

- **src/Domain/**: Core business logic (GimbalController, PIDController). Pure logic, minimal hardware dependency where possible.
- **src/Services/**: Application services (WebManager, WiFiManager, ConfigManager). These coordinate between the Domain and Infrastructure.
- **src/Infrastructure/**: Hardware interfaces (SensorManager).
- **data/**: Contains the LittleFS filesystem image.
    - `index.html`: The complete Single Page Application (Dashboard, Config, Help).
    - `config.json`: Runtime configuration storage.
- **include/config.h**: Contains **only** hardware pin definitions and compile-time constants.

### Working with Frontend
The frontend is in `esp32_firmware/data/index.html`. It is a Single Page Application using Tailwind CSS (CDN).
- To update the UI, edit `index.html`.
- **Important**: You must run `pio run --target uploadfs` to update the UI on the device.

### Configuration
- **Compile-time**: `include/config.h` (Pins, defaults).
- **Runtime**: `data/config.json`. Managed by `ConfigManager`.
- **Web Interface**: The `/api/config` endpoint allows reading/writing the JSON config.

## Verification
- Run `pio run` in `esp32_firmware/` to compile the firmware.
- Ensure `board_build.filesystem = littlefs` is present in `platformio.ini`.
