# PlatformIO Configuration Update for Raspberry Pi Pico W

## Summary

The Raspberry Pi Pico W version of the Solder Reflow Oven has been updated to use the official PlatformIO `raspberrypi` platform, ensuring better compatibility and long-term support.

## Changes Made

### 1. Updated `platformio.ini`

**Before:**
```ini
[env:pico_w]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico_w
framework = arduino
board_build.core = earlephilhower
```

**After:**
```ini
[env:pico_w]
platform = raspberrypi
board = rpipicow
framework = arduino
```

### Key Changes:
- **Platform**: Changed from custom GitHub repository to official `raspberrypi` platform
- **Board ID**: Changed from `pico_w` to `rpipicow` (correct board identifier)
- **Removed**: `board_build.core` setting (no longer needed with official platform)

## Why These Changes?

1. **Official Platform**: The `raspberrypi` platform is the official PlatformIO platform for RP2040 boards, providing:
   - Better long-term support
   - Regular updates and bug fixes
   - Consistent behavior across installations
   - Official documentation

2. **Correct Board ID**: The official board identifier is `rpipicow` for Raspberry Pi Pico W (with WiFi)

3. **Simplified Configuration**: The official platform handles core selection automatically, eliminating the need for `board_build.core`

## Verification

The configuration can be verified with:

```bash
cd "Electrical Design/SolderReflowOvenPicoW"
pio pkg list
```

This should output:
```
Platform raspberrypi @ 1.19.0+sha.317a978
├── framework-arduinopico @ 1.50501.0
├── tool-picotool-rp2040-earlephilhower @ 5.140200.250530
├── tool-pioasm-rp2040-earlephilhower @ 5.140200.250530
└── toolchain-rp2040-earlephilhower @ 5.140200.250530
```

## Building the Project

### Using PlatformIO CLI:

```bash
cd "Electrical Design/SolderReflowOvenPicoW"

# Build the project
pio run

# Upload to board (hold BOOTSEL button on first upload)
pio run --target upload

# Monitor serial output
pio device monitor
```

### Using VS Code with PlatformIO IDE:

1. Open the project folder in VS Code
2. PlatformIO will automatically detect the `platformio.ini`
3. Use the PlatformIO toolbar to Build, Upload, or Monitor

## Arduino IDE Compatibility

The original `.ino` file is still available for Arduino IDE users:
- `SolderReflowOvenPicoW/SolderReflowOvenPicoW.ino`
- Requires manual library installation
- Install "Raspberry Pi Pico/RP2040" boards package by Earle F. Philhower

## Features Preserved

All features of the Pico W version remain unchanged:

- **Dual-core architecture**: Core 0 handles WiFi/Web, Core 1 handles thermal control
- **10 Hz PID loop**: 10x faster than ESP8266 version
- **Separate SPI buses**: SPI0 for thermocouple, SPI1 reserved for future display
- **Web interface**: Real-time temperature monitoring and control
- **PID auto-tuning**: Automatic PID parameter optimization
- **WiFi modes**: Both Access Point and Station modes supported

## Dependencies

All dependencies are automatically managed by PlatformIO:

- Adafruit MAX31855 library v1.4.1
- PID v1.2.1 (br3ttb/PID)
- ArduinoJson v6.21.3

## Next Steps

1. **Test Build**: Build the project in your local environment
2. **Test Upload**: Upload to hardware and verify functionality
3. **Test Web Interface**: Access the web UI and test all features
4. **Run PID Auto-Tune**: Optimize PID parameters for your specific oven

## Troubleshooting

### Build Issues

If you encounter build issues:

1. **Update PlatformIO**: `pio upgrade`
2. **Clear cache**: `pio run --target clean`
3. **Reinstall platform**: `pio pkg uninstall -g -p raspberrypi && pio pkg install -g -p raspberrypi`

### Upload Issues

For first-time upload:

1. Hold down BOOTSEL button on Pico W
2. Connect USB cable while holding BOOTSEL
3. Release BOOTSEL
4. Pico W appears as USB mass storage device
5. Run `pio run --target upload`

For subsequent uploads, the bootloader is automatically activated.

## References

- [PlatformIO Raspberry Pi Platform Documentation](https://docs.platformio.org/en/latest/platforms/raspberrypi.html)
- [Arduino-Pico Core by Earle F. Philhower](https://github.com/earlephilhower/arduino-pico)
- [PLATFORMIO_SETUP.md](PLATFORMIO_SETUP.md) - General PlatformIO setup guide
- [PICO_W_MIGRATION_GUIDE.md](PICO_W_MIGRATION_GUIDE.md) - Migration from ESP8266 to Pico W
- [README_PICOW.md](Electrical%20Design/SolderReflowOvenPicoW/README_PICOW.md) - Pico W specific documentation

## Support

If you encounter any issues with the PlatformIO configuration:

1. Check the [PlatformIO documentation](https://docs.platformio.org/)
2. Review the [Arduino-Pico documentation](https://arduino-pico.readthedocs.io/)
3. Open an issue on GitHub with details about your environment and error messages
