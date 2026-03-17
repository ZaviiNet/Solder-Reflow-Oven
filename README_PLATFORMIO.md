# ESP8266 Reflow Oven - PlatformIO Project

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
- `src/main.cpp` - Main source file (converted from SolderReflowOven.ino)
- `SolderReflowOven.ino` - Original Arduino file (kept for reference/Arduino IDE compatibility)
- `lib/` - Custom libraries (empty for now)
- `include/` - Header files (empty for now)
- `test/` - Unit tests (empty for now)

## Using Arduino IDE

You can still use the original `SolderReflowOven.ino` file with Arduino IDE:

1. Open Arduino IDE
2. Open `SolderReflowOven.ino`
3. Install required libraries manually:
   - Adafruit MAX31855 library
   - PID by Brett Beauregard
   - ArduinoJson
   - WebSockets by Markus Sattler
4. Select board: NodeMCU 1.0 (ESP-12E Module)
5. Upload

## Library Dependencies

All dependencies are automatically managed by PlatformIO:

- Adafruit MAX31855 library v1.4.1
- PID v1.2.1
- ArduinoJson v6.21.3
- WebSockets v2.4.1

## Hardware

- NodeMCU 1.0 ESP12E (ESP8266)
- Adafruit MAX31855 K-Type Thermocouple Amplifier
- Solid State Relay (SSR) for heater control

## Pin Connections

- D5 (GPIO14): MAX31855 CLK
- D4 (GPIO2): MAX31855 CS
- D6 (GPIO12): MAX31855 DO (MISO)
- D8 (GPIO15): SSR Control

## More Information

See the main [README.md](../../README.md) and [PLATFORMIO_SETUP.md](../../PLATFORMIO_SETUP.md) for more details.
