# CLion + PlatformIO Setup Guide

## Overview

This project is configured to work seamlessly with JetBrains CLion and PlatformIO. CLion provides excellent code completion, navigation, and debugging features while PlatformIO handles building, dependencies, and uploading.

## Prerequisites

### 1. Install PlatformIO Core

```bash
# Install PlatformIO Core CLI
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o get-platformio.py
python3 get-platformio.py

# Or via pip
pip install -U platformio

# Verify installation
pio --version
```

### 2. Install CLion

Download and install CLion from: https://www.jetbrains.com/clion/

## Quick Start

### Method 1: Using CLion PlatformIO Plugin (Recommended)

1. **Install PlatformIO Plugin in CLion**:
   - Open CLion
   - Go to `Settings/Preferences → Plugins`
   - Search for "PlatformIO"
   - Click `Install` and restart CLion

2. **Open Project**:
   - `File → Open`
   - Select the `Solder-Reflow-Oven-IO` directory
   - CLion will detect the `platformio.ini` file

3. **Build and Upload**:
   - Use the PlatformIO toolbar in CLion
   - Or use PlatformIO menu: `Tools → PlatformIO → Build` / `Upload`

### Method 2: Using Terminal (Works with any CLion version)

1. **Open Project in CLion**:
   - `File → Open`
   - Select the `Solder-Reflow-Oven-IO` directory

2. **Use Built-in Terminal** (`View → Tool Windows → Terminal`):
   ```bash
   # Build for ESP8266
   pio run -e nodemcuv2

   # Build for Raspberry Pi Pico W
   pio run -e pico

   # Upload to board
   pio run -e nodemcuv2 -t upload

   # Monitor serial output
   pio device monitor -b 115200

   # Clean build
   pio run -t clean
   ```

## Project Structure

```
Solder-Reflow-Oven-IO/
├── platformio.ini          # PlatformIO configuration (MAIN CONFIG)
├── CMakeLists.txt          # CLion integration (for IDE features only)
├── src/
│   └── main.cpp           # Main source file
├── include/               # Header files
├── lib/                   # Custom libraries
├── test/                  # Unit tests
└── .pio/                  # PlatformIO build artifacts (auto-generated)
```

## CLion Features

### Code Completion and Navigation

CLion provides intelligent code completion for:
- ✅ Arduino framework functions
- ✅ PlatformIO libraries
- ✅ All installed dependencies (PID, Adafruit, etc.)

**Usage:**
- `Ctrl+Space` - Code completion
- `Ctrl+Click` or `Ctrl+B` - Go to definition
- `Ctrl+Alt+Left/Right` - Navigate back/forward
- `Ctrl+Shift+N` - Find file
- `Ctrl+Shift+F` - Find in files

### Refactoring

- `Shift+F6` - Rename symbol
- `Ctrl+Alt+M` - Extract method
- `Ctrl+Alt+V` - Extract variable
- `Ctrl+Alt+C` - Extract constant

### Building and Uploading

**Using Terminal** (Most reliable):
```bash
# Build
pio run -e nodemcuv2

# Upload
pio run -e nodemcuv2 -t upload

# Monitor
pio device monitor
```

**Using Run Configurations** (Advanced):
1. `Run → Edit Configurations`
2. Add new `Shell Script` configuration
3. Set script: `pio run -e nodemcuv2 -t upload`
4. Click `Run` button in toolbar

## PlatformIO Environments

The project supports multiple boards via environments:

### 1. ESP8266 NodeMCU v2 (Default)
```bash
pio run -e nodemcuv2
pio run -e nodemcuv2 -t upload
```

**Features:**
- WiFi enabled
- Web server
- WebSocket support
- Auto-tune PID

### 2. Raspberry Pi Pico W
```bash
pio run -e pico
pio run -e pico -t upload
```

**Features:**
- Dual-core processing
- 10x faster PID loop (10 Hz vs 1 Hz)
- Better thermal control
- See [PICO_W_MIGRATION_GUIDE.md](PICO_W_MIGRATION_GUIDE.md)

## Common Commands

### Building
```bash
# Build default environment
pio run

# Build specific environment
pio run -e nodemcuv2
pio run -e pico

# Clean build
pio run -t clean

# Verbose output
pio run -v
```

### Uploading
```bash
# Upload to board
pio run -t upload

# Specify upload port
pio run -t upload --upload-port /dev/ttyUSB0

# Upload and monitor
pio run -t upload && pio device monitor
```

### Monitoring
```bash
# Open serial monitor
pio device monitor

# Specify baud rate
pio device monitor -b 115200

# List available ports
pio device list
```

### Dependencies
```bash
# Update all libraries
pio lib update

# List installed libraries
pio lib list

# Install new library
pio lib install "library-name"
```

## Troubleshooting

### Issue: CLion can't find includes

**Solution 1**: Generate CMake helper files
```bash
pio init --ide clion
```

**Solution 2**: Reload CMake project
- `File → Reload CMake Project`

**Solution 3**: Invalidate caches
- `File → Invalidate Caches / Restart`

### Issue: Upload fails

**Check:**
1. Board is connected via USB
2. Correct port permissions: `sudo usermod -a -G dialout $USER` (logout/login required)
3. Correct environment selected
4. Driver installed (CH340/CP2102 for ESP8266)

**Test:**
```bash
# List devices
pio device list

# Try manual port
pio run -t upload --upload-port /dev/ttyUSB0
```

### Issue: Build errors after switching environments

**Solution:**
```bash
# Clean and rebuild
pio run -t clean
pio run -e nodemcuv2
```

### Issue: Library not found

**Solution:**
```bash
# Update libraries
pio lib update

# Reinstall libraries
pio lib install
```

## Development Workflow

### Recommended Workflow

1. **Edit Code** in CLion (use all IDE features)
2. **Build** via terminal: `pio run -e nodemcuv2`
3. **Fix errors** using CLion's error navigation
4. **Upload** via terminal: `pio run -e nodemcuv2 -t upload`
5. **Monitor** via terminal: `pio device monitor`

### Testing Changes

```bash
# Quick test cycle
pio run -e nodemcuv2 && pio run -e nodemcuv2 -t upload && pio device monitor
```

### Multi-Platform Development

```bash
# Test on ESP8266
pio run -e nodemcuv2

# Test on Pico W
pio run -e pico

# Ensure compatibility
pio run  # Builds all environments
```

## Advanced Features

### Custom Build Flags

Edit `platformio.ini`:
```ini
[env:nodemcuv2]
build_flags =
    -D DEBUG_MODE=1
    -D WIFI_TIMEOUT=30000
```

### Unit Testing

Create tests in `test/` directory:
```bash
# Run tests
pio test

# Run tests for specific environment
pio test -e nodemcuv2
```

### OTA Updates (ESP8266 only)

Enable in `platformio.ini`:
```ini
upload_protocol = espota
upload_port = 192.168.4.1
```

Then: `pio run -e nodemcuv2 -t upload`

## Additional Resources

### Documentation
- [PlatformIO CLI](https://docs.platformio.org/en/latest/core/userguide/index.html)
- [PlatformIO IDE Integration](https://docs.platformio.org/en/latest/integration/ide/clion.html)
- [ESP8266 Platform](https://docs.platformio.org/en/latest/platforms/espressif8266.html)
- [Raspberry Pi Platform](https://docs.platformio.org/en/latest/platforms/raspberrypi.html)

### Project Documentation
- [README.md](README.md) - Main project documentation
- [README_PLATFORMIO.md](README_PLATFORMIO.md) - PlatformIO overview
- [PICO_W_MIGRATION_GUIDE.md](PICO_W_MIGRATION_GUIDE.md) - Migrating to Pico W
- [AUTO_TUNE_FEATURE.md](AUTO_TUNE_FEATURE.md) - PID auto-tuning guide
- [PID_TUNING_GUIDE.md](PID_TUNING_GUIDE.md) - Complete PID tuning guide

## Tips and Tricks

### 1. Speed up builds
```bash
# Use ccache
export PLATFORMIO_BUILD_FLAGS="-j4"
```

### 2. Quick serial monitor access
Add to your shell profile:
```bash
alias piomonitor='pio device monitor -b 115200'
```

### 3. CLion External Tools
Add PlatformIO commands as external tools:
- `Settings → Tools → External Tools → Add`
- Name: "PlatformIO Upload"
- Program: `pio`
- Arguments: `run -e nodemcuv2 -t upload`
- Working directory: `$ProjectFileDir$`

### 4. Code formatting
```bash
# Format all files (uses .clang-format)
find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

## Conclusion

With CLion and PlatformIO, you get:
- ✅ Professional IDE features (completion, navigation, refactoring)
- ✅ Automatic dependency management
- ✅ Cross-platform builds (ESP8266 and Pico W)
- ✅ Easy library updates
- ✅ Integrated testing framework

**Happy coding! 🚀**

