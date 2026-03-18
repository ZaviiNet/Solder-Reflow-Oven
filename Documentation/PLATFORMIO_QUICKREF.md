# PlatformIO Quick Reference

Quick reference for building and uploading firmware using PlatformIO.

## Installation

```bash
# Install PlatformIO CLI
pip install platformio

# Or use VS Code extension
# Search for "PlatformIO IDE" in VS Code extensions
```

## Basic Commands

### Build
```bash
pio run                    # Build project
pio run -v                 # Build with verbose output
pio run --target clean     # Clean build files
```

### Upload
```bash
pio run --target upload    # Upload to board
```

### Monitor
```bash
pio device monitor         # Open serial monitor
pio device list            # List connected devices
```

### Combined
```bash
# Build, upload, and monitor in one command
pio run --target upload && pio device monitor
```

## Project Locations

### ESP8266 NodeMCU
```bash
cd "Electrical Design/SolderReflowOven"
pio run --target upload
```

### Raspberry Pi Pico W
```bash
cd "Electrical Design/SolderReflowOvenPicoW"
pio run --target upload
```

### MAX31855 Example
```bash
cd "Electrical Design/ESP8266_MAX31855_Example"
pio run --target upload
```

## Library Management

```bash
pio lib install <library>  # Install library
pio lib search <keyword>   # Search for library
pio lib update             # Update all libraries
pio lib list               # List installed libraries
```

## Configuration

Edit `platformio.ini` in each project directory:

```ini
[env:board]
platform = ...
board = ...
framework = arduino
monitor_speed = 115200
lib_deps = 
    library1 @ ^1.0.0
    library2 @ ^2.0.0
```

## Troubleshooting

### Permission Error (Linux)
```bash
sudo usermod -a -G dialout $USER
# Logout and login again
```

### Upload Port
```bash
# Specify port manually
pio run --target upload --upload-port /dev/ttyUSB0  # Linux
pio run --target upload --upload-port COM3         # Windows
```

### Clean Build
```bash
pio run --target clean
rm -rf .pio
pio run
```

## More Information

- Full guide: [PLATFORMIO_SETUP.md](PLATFORMIO_SETUP.md)
- Migration info: [PLATFORMIO_MIGRATION.md](PLATFORMIO_MIGRATION.md)
- PlatformIO docs: https://docs.platformio.org/
