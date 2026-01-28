#!/bin/bash

# Validation script for ESP32 3-Axis Gimbal Control System

echo "========================================"
echo "ESP32 Gimbal System Validation"
echo "========================================"
echo ""

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Track overall status
ALL_PASSED=true

# Function to check if a file exists
check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} Found: $1"
        return 0
    else
        echo -e "${RED}✗${NC} Missing: $1"
        ALL_PASSED=false
        return 1
    fi
}

# Function to check if a directory exists
check_dir() {
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} Found directory: $1"
        return 0
    else
        echo -e "${RED}✗${NC} Missing directory: $1"
        ALL_PASSED=false
        return 1
    fi
}

echo "Checking ESP32 Firmware Files..."
check_file "esp32_firmware/platformio.ini"
check_file "esp32_firmware/src/main.cpp"
check_file "esp32_firmware/include/config.h"
echo ""

echo "Checking Backend Files..."
check_file "backend/main.py"
check_file "backend/requirements.txt"
check_file "backend/app/models.py"
echo ""

echo "Checking Documentation..."
check_file "README.md"
check_file "docs/QUICKSTART.md"
check_file "docs/HARDWARE_SETUP.md"
check_file "docs/API.md"
check_file "docs/ARCHITECTURE.md"
check_file "docs/DEPLOYMENT.md"
echo ""

echo "Checking Configuration Files..."
check_file ".gitignore"
check_file "config.example.json"
check_file "LICENSE"
check_file "CONTRIBUTING.md"
check_file "CHANGELOG.md"
echo ""

echo "Checking Project Structure..."
check_dir "esp32_firmware/src"
check_dir "esp32_firmware/include"
check_dir "backend/app"
check_dir "docs"
echo ""

# Check for PlatformIO
echo "Checking Dependencies..."
if command -v pio &> /dev/null; then
    echo -e "${GREEN}✓${NC} PlatformIO is installed"
else
    echo -e "${YELLOW}⚠${NC} PlatformIO not found (install with: pip install platformio)"
fi

# Check for Python
if command -v python3 &> /dev/null; then
    echo -e "${GREEN}✓${NC} Python 3 is installed"
else
    echo -e "${YELLOW}⚠${NC} Python 3 not found"
fi

echo ""
echo "========================================"
if [ "$ALL_PASSED" = true ]; then
    echo -e "${GREEN}✓ All validation checks passed!${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Configure WiFi in esp32_firmware/include/config.h"
    echo "2. Upload firmware: cd esp32_firmware && pio run --target upload"
    echo "3. Connect to gimbal via WiFi or hotspot mode"
    echo "4. Access web interface in your browser"
    echo ""
    echo "See docs/QUICKSTART.md for detailed instructions."
    exit 0
else
    echo -e "${RED}✗ Some validation checks failed!${NC}"
    echo "Please ensure all required files are present."
    exit 1
fi
