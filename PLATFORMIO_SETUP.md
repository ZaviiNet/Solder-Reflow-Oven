# PlatformIO Setup Guide

This repository has been migrated to use PlatformIO for a better development experience. The Arduino .ino files are still present for compatibility, but PlatformIO is now the recommended build system.

## Why PlatformIO?

- **Better dependency management**: Automatic library installation and version management
- **Cross-platform**: Works on Windows, Mac, and Linux
- **Multiple boards**: Easy to manage different hardware targets
- **Advanced features**: Unit testing, static analysis, remote development
- **IDE integration**: Works with VS Code, CLion, Atom, and more
- **Command-line friendly**: Build and upload from terminal
- **Library management**: No manual library installation needed

## Installation

### Option 1: PlatformIO Core (CLI)

```bash
# Install using pip
pip install platformio

# Or using homebrew (macOS)
brew install platformio
```

### Option 2: PlatformIO IDE (VS Code)

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code
3. Go to Extensions (Ctrl+Shift+X)
4. Search for "PlatformIO IDE"
5. Click Install

## Project Structure

Each project now has the following structure:

```
SolderReflowOven/
├── platformio.ini          # PlatformIO configuration
├── src/
│   └── main.cpp           # Main source file (converted from .ino)
├── lib/                   # Custom libraries (empty for now)
├── include/               # Header files (empty for now)
├── test/                  # Unit tests (empty for now)
└── SolderReflowOven.ino  # Original Arduino file (kept for reference)
```

## Available Projects

### 1. ESP8266 NodeMCU Version
**Location**: `Electrical Design/SolderReflowOven/`

```bash
cd "Electrical Design/SolderReflowOven"

# Build the project
pio run

# Upload to board
pio run --target upload

# Monitor serial output
pio device monitor
```

### 2. Raspberry Pi Pico W Version
**Location**: `Electrical Design/SolderReflowOvenPicoW/`

```bash
cd "Electrical Design/SolderReflowOvenPicoW"

# Build the project
pio run

# Upload to board
pio run --target upload

# Monitor serial output
pio device monitor
```

### 3. MAX31855 Example
**Location**: `Electrical Design/ESP8266_MAX31855_Example/`

```bash
cd "Electrical Design/ESP8266_MAX31855_Example"

# Build the project
pio run

# Upload to board
pio run --target upload

# Monitor serial output
pio device monitor
```

## Basic Commands

### Building
```bash
# Build project
pio run

# Clean build files
pio run --target clean

# Build in verbose mode
pio run -v
```

### Uploading
```bash
# Upload firmware
pio run --target upload

# Upload and monitor
pio run --target upload && pio device monitor

# Specify upload port
pio run --target upload --upload-port /dev/ttyUSB0
```

### Serial Monitor
```bash
# Open serial monitor
pio device monitor

# With specific baud rate
pio device monitor -b 115200

# List available devices
pio device list
```

### Library Management
```bash
# Install a library
pio lib install "library-name"

# Search for libraries
pio lib search "keyword"

# Update all libraries
pio lib update

# Show installed libraries
pio lib list
```

## Configuration

Each project has a `platformio.ini` file that defines:

- **Platform**: espressif8266 or raspberrypi
- **Board**: nodemcuv2, pico_w, etc.
- **Framework**: arduino
- **Libraries**: Automatic dependency resolution
- **Build flags**: Compiler options
- **Upload settings**: Speed and port

Example `platformio.ini` for ESP8266:
```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 115200
lib_deps = 
    adafruit/Adafruit MAX31855 library @ ^1.4.1
    br3ttb/PID @ ^1.2.1
    bblanchon/ArduinoJson @ ^6.21.3
    links2004/WebSockets @ ^2.4.1
```

## IDE Integration

### VS Code (Recommended)

1. Install PlatformIO IDE extension
2. Open project folder in VS Code
3. PlatformIO will auto-detect `platformio.ini`
4. Use the PlatformIO toolbar at the bottom
5. Click icons to build, upload, monitor, etc.

### CLion

1. Install PlatformIO plugin
2. Import project
3. Use PlatformIO run configurations

## Migrating from Arduino IDE

The Arduino .ino files are still present in the repository for compatibility:

- `SolderReflowOven.ino` → `src/main.cpp`
- `SolderReflowOvenPicoW.ino` → `src/main.cpp`
- `ESP8266_MAX31855_Example.ino` → `src/main.cpp`

**Note**: PlatformIO uses `src/main.cpp` instead of `.ino` files, but the code is identical.

### Differences

1. **File extension**: `.ino` → `.cpp`
2. **Library installation**: Automatic via `platformio.ini`
3. **Board selection**: Defined in `platformio.ini`
4. **No forward declarations needed**: PlatformIO handles this automatically

## Troubleshooting

### "platformio: command not found"

If PlatformIO is not in your PATH:

```bash
# Linux/Mac - Add to ~/.bashrc or ~/.zshrc
export PATH=$PATH:~/.local/bin

# Or use full path
~/.local/bin/pio run
```

### Upload Issues

```bash
# Check connected devices
pio device list

# Specify upload port
pio run --target upload --upload-port /dev/ttyUSB0  # Linux
pio run --target upload --upload-port COM3         # Windows
```

### Library Not Found

```bash
# Clean and rebuild
pio run --target clean
pio lib install  # Reinstall dependencies
pio run
```

### Permission Denied (Linux)

```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER

# Then logout and login again
```

## Advanced Features

### Unit Testing

```bash
# Run unit tests
pio test

# Run specific test
pio test -f test_name
```

### Remote Development

```bash
# Upload and run remotely
pio remote run --target upload
```

### Static Analysis

```bash
# Run static code analysis
pio check
```

### OTA (Over-The-Air) Updates

Configure in `platformio.ini`:
```ini
upload_protocol = espota
upload_port = 192.168.1.100
upload_flags =
    --auth=password
```

## Additional Resources

- [PlatformIO Documentation](https://docs.platformio.org/)
- [PlatformIO Registry](https://registry.platformio.org/) - Library search
- [PlatformIO Community](https://community.platformio.org/)
- [Arduino Migration Guide](https://docs.platformio.org/en/latest/tutorials/ststm32/arduino_debugging_unit_testing.html)

## Support

If you encounter issues with PlatformIO:

1. Check this guide first
2. Review [PlatformIO Documentation](https://docs.platformio.org/)
3. Search [PlatformIO Community Forum](https://community.platformio.org/)
4. Open an issue in this repository

## Original Arduino IDE Support

The original Arduino .ino files are still maintained and can be used with Arduino IDE:

1. Open Arduino IDE
2. Navigate to the project folder
3. Open the `.ino` file
4. Install required libraries manually
5. Select the correct board
6. Upload as usual

However, PlatformIO is now the recommended development environment for this project.
