#!/bin/bash
# ESP32 3-Axis Gimbal - Quick Deployment Script
# This script automates the firmware and filesystem upload process

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
FIRMWARE_DIR="$SCRIPT_DIR/esp32_firmware"

echo -e "${BLUE}╔════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  ESP32 3-Axis Gimbal Deployment Script   ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════╝${NC}"
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo -e "${RED}❌ PlatformIO not found!${NC}"
    echo -e "Install it with: ${YELLOW}pip install platformio${NC}"
    exit 1
fi

echo -e "${GREEN}✓${NC} PlatformIO found"

# Change to firmware directory
if [ ! -d "$FIRMWARE_DIR" ]; then
    echo -e "${RED}❌ Firmware directory not found: $FIRMWARE_DIR${NC}"
    exit 1
fi

cd "$FIRMWARE_DIR"
echo -e "${GREEN}✓${NC} Changed to firmware directory"
echo ""

# Function to detect serial port
detect_port() {
    if [ "$(uname)" == "Darwin" ]; then
        # macOS
        PORT=$(ls /dev/cu.* 2>/dev/null | grep -E 'usbserial|SLAB|wchusbserial' | head -n 1)
    elif [[ "$(uname -s)" == Linux* ]]; then
        # Linux
        PORT=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -n 1)
    else
        # Windows (Git Bash/MSYS)
        PORT="COM3"  # Default, user should override if needed
    fi
    
    if [ -z "$PORT" ]; then
        echo -e "${YELLOW}⚠ Could not auto-detect serial port${NC}"
        return 1
    fi
    
    echo "$PORT"
    return 0
}

# Parse command line arguments
ACTION="${1:-all}"
PORT_ARG="${2:-}"

# Detect or use provided port
if [ -n "$PORT_ARG" ]; then
    PORT="$PORT_ARG"
    echo -e "${BLUE}ℹ Using specified port: $PORT${NC}"
elif DETECTED_PORT=$(detect_port); then
    PORT="$DETECTED_PORT"
    echo -e "${GREEN}✓${NC} Auto-detected port: ${GREEN}$PORT${NC}"
else
    echo -e "${YELLOW}Please specify port as second argument:${NC}"
    echo -e "  ${YELLOW}./deploy.sh all /dev/ttyUSB0${NC}  (Linux/Mac)"
    echo -e "  ${YELLOW}./deploy.sh all COM3${NC}          (Windows)"
    exit 1
fi

echo ""

# Build firmware
build_firmware() {
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  Building Firmware${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    pio run
    echo -e "${GREEN}✓ Build complete${NC}"
    echo ""
}

# Upload firmware
upload_firmware() {
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  Uploading Firmware to $PORT${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    pio run --target upload --upload-port "$PORT"
    echo -e "${GREEN}✓ Firmware uploaded${NC}"
    echo ""
}

# Upload filesystem
upload_filesystem() {
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  Uploading Filesystem (Web GUI) to $PORT${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}This uploads: index.html, config.json, favicon.svg${NC}"
    pio run --target uploadfs --upload-port "$PORT"
    echo -e "${GREEN}✓ Filesystem uploaded${NC}"
    echo ""
}

# Monitor serial output
monitor_serial() {
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  Serial Monitor (Ctrl+C to exit)${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    pio device monitor --port "$PORT" --baud 115200
}

# Main execution
case "$ACTION" in
    "build")
        build_firmware
        ;;
    "upload")
        upload_firmware
        ;;
    "uploadfs"|"filesystem")
        upload_filesystem
        ;;
    "monitor")
        monitor_serial
        ;;
    "all"|"deploy")
        build_firmware
        upload_firmware
        upload_filesystem
        echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo -e "${GREEN}  ✓ Deployment Complete!${NC}"
        echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo ""
        echo -e "${BLUE}Next steps:${NC}"
        echo -e "  1. Wait 5-10 seconds for ESP32 to boot"
        echo -e "  2. Connect to WiFi AP: ${YELLOW}Gimbal_AP${NC}"
        echo -e "  3. Open browser: ${YELLOW}http://192.168.4.1${NC}"
        echo -e "  4. For Bluetooth: Use a ${YELLOW}BLE scanner app${NC} (LightBlue/nRF Connect)"
        echo ""
        echo -e "To monitor serial output:"
        echo -e "  ${YELLOW}./deploy.sh monitor $PORT${NC}"
        echo ""
        ;;
    "help"|"--help"|"-h")
        echo "Usage: $0 [action] [port]"
        echo ""
        echo "Actions:"
        echo "  all|deploy   - Build, upload firmware, upload filesystem (default)"
        echo "  build        - Build firmware only"
        echo "  upload       - Upload firmware only"
        echo "  uploadfs     - Upload filesystem (web GUI) only"
        echo "  monitor      - Open serial monitor"
        echo "  help         - Show this help"
        echo ""
        echo "Examples:"
        echo "  $0                          # Full deploy (auto-detect port)"
        echo "  $0 all /dev/ttyUSB0         # Full deploy to specific port"
        echo "  $0 uploadfs /dev/ttyUSB0    # Upload filesystem only"
        echo "  $0 monitor COM3             # Monitor serial output (Windows)"
        echo ""
        ;;
    *)
        echo -e "${RED}Unknown action: $ACTION${NC}"
        echo "Run '$0 help' for usage information"
        exit 1
        ;;
esac
