# ESP8266 MAX31855 Example - PlatformIO Project

Simple example project for testing MAX31855 thermocouple with ESP8266.

## Quick Start with PlatformIO

```bash
# Build the project
pio run

# Upload to board
pio run --target upload

# Monitor serial output
pio device monitor
```

## Project Structure

- `platformio.ini` - PlatformIO configuration file
- `src/main.cpp` - Main source file (converted from ESP8266_MAX31855_Example.ino)
- `ESP8266_MAX31855_Example.ino` - Original Arduino file (kept for reference)

## Using Arduino IDE

You can still use the original `ESP8266_MAX31855_Example.ino` file with Arduino IDE:

1. Open Arduino IDE
2. Open `ESP8266_MAX31855_Example.ino`
3. Install Adafruit MAX31855 library
4. Select board: NodeMCU 1.0 (ESP-12E Module)
5. Upload

## Library Dependencies

- Adafruit MAX31855 library v1.4.1 (automatically managed by PlatformIO)

## More Information

See the main [README.md](../../README.md) and [PLATFORMIO_SETUP.md](../../PLATFORMIO_SETUP.md) for more details.
