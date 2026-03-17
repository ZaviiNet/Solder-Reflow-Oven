# Raspberry Pi Pico W Reflow Oven - PlatformIO Project

This project has been converted to PlatformIO format for easier dependency management and building.

## Quick Start with PlatformIO

```bash
# Build the project
pio run

# Upload to board
pio run --target upload

# Monitor serial output
pio device monitor

# Or do all at once
pio run --target upload && pio device monitor
```

## Project Structure

- `platformio.ini` - PlatformIO configuration file
- `src/main.cpp` - Main source file (converted from SolderReflowOvenPicoW.ino)
- `SolderReflowOvenPicoW.ino` - Original Arduino file (kept for reference/Arduino IDE compatibility)
- `lib/` - Custom libraries (empty for now)
- `include/` - Header files (empty for now)
- `test/` - Unit tests (empty for now)

## Using Arduino IDE

You can still use the original `SolderReflowOvenPicoW.ino` file with Arduino IDE:

1. Install Raspberry Pi Pico/RP2040 board support (Earl E. Philhower's core)
2. Open Arduino IDE
3. Open `SolderReflowOvenPicoW.ino`
4. Install required libraries manually:
   - Adafruit MAX31855 library
   - PID by Brett Beauregard
   - ArduinoJson
5. Select board: Raspberry Pi Pico W
6. Upload

## Library Dependencies

All dependencies are automatically managed by PlatformIO:

- Adafruit MAX31855 library v1.4.1
- PID v1.2.1
- ArduinoJson v6.21.3

## Hardware

- Raspberry Pi Pico W (RP2040 dual-core @ 133 MHz)
- Adafruit MAX31855 K-Type Thermocouple Amplifier
- Solid State Relay (SSR) for heater control

## Pin Connections

**SPI0 (Thermocouple - Core 1):**
- GPIO 18: MAX31855 CLK (SCK)
- GPIO 16: MAX31855 DO (MISO)
- GPIO 17: MAX31855 CS

**Other:**
- GPIO 15: SSR Control
- GPIO 25: Onboard LED (status indicator)

## Dual-Core Architecture

- **Core 0**: WiFi, Web Server, WebSocket communication
- **Core 1**: Temperature monitoring, PID control, SSR control (10 Hz loop)

This provides 10x faster PID control compared to ESP8266 version!

## More Information

See the main [README.md](../../README.md), [README_PICOW.md](README_PICOW.md), and [PLATFORMIO_SETUP.md](../../PLATFORMIO_SETUP.md) for more details.
