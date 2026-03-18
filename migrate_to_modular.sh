#!/bin/bash
# Migration script for refactored code structure
# This script backs up the old main.cpp and activates the new modular structure

set -e  # Exit on error

PROJECT_DIR="/home/jack/CLionProjects/Solder-Reflow-Oven-IO"
SRC_DIR="$PROJECT_DIR/src"

echo "================================================"
echo "Solder Reflow Oven - Code Refactoring Migration"
echo "================================================"
echo ""

# Check if we're in the right directory
if [ ! -d "$SRC_DIR" ]; then
    echo "ERROR: Source directory not found: $SRC_DIR"
    exit 1
fi

# Check if files exist
if [ ! -f "$SRC_DIR/main.cpp" ]; then
    echo "ERROR: main.cpp not found"
    exit 1
fi

if [ ! -f "$SRC_DIR/main_new.cpp" ]; then
    echo "ERROR: main_new.cpp not found"
    exit 1
fi

echo "Step 1: Creating backup of original main.cpp..."
cp "$SRC_DIR/main.cpp" "$SRC_DIR/main_old_backup_$(date +%Y%m%d_%H%M%S).cpp"
echo "  ✓ Backup created"

echo ""
echo "Step 2: Replacing main.cpp with refactored version..."
mv "$SRC_DIR/main.cpp" "$SRC_DIR/main_old.cpp"
mv "$SRC_DIR/main_new.cpp" "$SRC_DIR/main.cpp"
echo "  ✓ main.cpp replaced"

echo ""
echo "Step 3: Verifying all module files exist..."
modules=(
    "pins.h"
    "config.h"
    "temperature.h"
    "temperature.cpp"
    "pid_controller.h"
    "pid_controller.cpp"
    "led_control.h"
    "led_control.cpp"
    "storage.h"
    "storage.cpp"
    "wifi_setup.h"
    "wifi_setup.cpp"
    "web_server.h"
    "web_server.cpp"
    "state_machine.h"
    "state_machine.cpp"
)

missing_files=()
for module in "${modules[@]}"; do
    if [ ! -f "$SRC_DIR/$module" ]; then
        missing_files+=("$module")
    fi
done

if [ ${#missing_files[@]} -eq 0 ]; then
    echo "  ✓ All module files present"
else
    echo "  ✗ Missing files:"
    for file in "${missing_files[@]}"; do
        echo "    - $file"
    done
    exit 1
fi

echo ""
echo "================================================"
echo "Migration Complete!"
echo "================================================"
echo ""
echo "Summary:"
echo "  - Old main.cpp saved as: main_old.cpp"
echo "  - Backup also saved with timestamp"
echo "  - New modular structure is now active"
echo ""
echo "Next steps:"
echo "  1. Build the project: pio run"
echo "  2. Upload to device: pio run --target upload"
echo "  3. Monitor serial: pio device monitor"
echo ""
echo "To rollback (if needed):"
echo "  mv $SRC_DIR/main.cpp $SRC_DIR/main_new.cpp"
echo "  mv $SRC_DIR/main_old.cpp $SRC_DIR/main.cpp"
echo ""

