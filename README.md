# Solder Reflow Oven Controller

This repository contains complete reflow oven controllers for multiple platforms with web-based interfaces.

## 🚀 NEW: PlatformIO Support!

**Now using PlatformIO for better development experience!**

- **Easy setup**: No manual library installation
- **Better dependency management**: Automatic library version control
- **Cross-platform**: Works on Windows, Mac, and Linux
- **IDE integration**: VS Code, CLion, Atom support
- See **[Documentation/PLATFORMIO_SETUP.md](Documentation/PLATFORMIO_SETUP.md)** for complete setup guide

**Note**: Arduino .ino files are still available for Arduino IDE users.

## 🆕 NEW: Raspberry Pi Pico W Dual-Core Version!

**Upgrade to Pico W for 10x faster PID control!**

- **Dual-core architecture**: Dedicated core for thermal control
- **10x faster PID loop**: 10 Hz vs 1 Hz on ESP8266
- **Better temperature control**: Less overshoot, smoother curves
- **Separate SPI buses**: Thermocouple + future display support
- See **[PICO_W_MIGRATION_GUIDE.md](PICO_W_MIGRATION_GUIDE.md)** for upgrade instructions

## Available Versions

**Not sure which to choose?** See **[PLATFORM_COMPARISON.md](PLATFORM_COMPARISON.md)** for detailed comparison.

### 1. Raspberry Pi Pico W (Recommended - Dual-Core)
- **Location**: `Electrical Design/SolderReflowOvenPicoW/`
- **Processor**: RP2040 dual-core @ 133 MHz
- **PID Loop**: 10 Hz (100ms updates)
- **Benefits**: Faster response, dedicated thermal core, better performance

### 2. ESP8266 NodeMCU (Original)
- **Location**: `Electrical Design/SolderReflowOven/`
- **Processor**: ESP8266 @ 80 MHz
- **PID Loop**: 1 Hz (1 second updates)
- **Status**: Stable, fully functional

## Quick Start

### Using PlatformIO (Recommended):
```bash
# Install PlatformIO
pip install platformio

# For ESP8266
cd "Electrical Design/SolderReflowOven"
pio run --target upload

# For Pico W
cd "Electrical Design/SolderReflowOvenPicoW"
pio run --target upload
```

See **[Documentation/PLATFORMIO_SETUP.md](Documentation/PLATFORMIO_SETUP.md)** for detailed instructions.

### Using Arduino IDE (Alternative):

#### For Raspberry Pi Pico W:
1. **Hardware**: Raspberry Pi Pico W + MAX31855 Thermocouple + SSR
2. **Software**: Arduino IDE with Pico board support
3. **Docs**: See `Electrical Design/SolderReflowOvenPicoW/README_PICOW.md`
4. **Setup**: Upload code, connect to "ReflowOven" WiFi
5. **Access**: Navigate to http://192.168.4.1
6. **Use**: Configure profile, click START REFLOW

#### For ESP8266 NodeMCU:
1. **Hardware**: NodeMCU ESP8266 + MAX31855 Thermocouple + SSR
2. **Software**: Arduino IDE with ESP8266 board support
3. **Setup**: Upload code, connect to "ReflowOven" WiFi
4. **Access**: Navigate to http://192.168.4.1
5. **Use**: Configure profile, click START REFLOW

## Documentation

**All documentation files are located in the [Documentation/](Documentation/) folder.**

### Quick Start

- **[Documentation/START_HERE.md](Documentation/START_HERE.md)** - Start here for setup and first steps
  - Quick setup guide
  - Hardware assembly
  - First reflow walkthrough

- **[Documentation/QUICK_REFERENCE.md](Documentation/QUICK_REFERENCE.md)** - Quick reference card
  - Key commands and shortcuts
  - API endpoints summary
  - Temperature profiles at a glance
  - Troubleshooting quick tips

### Development Environment Setup

- **[Documentation/PLATFORMIO_SETUP.md](Documentation/PLATFORMIO_SETUP.md)** - PlatformIO setup and usage guide
  - Why PlatformIO
  - Installation instructions
  - Building and uploading
  - IDE integration
  - Troubleshooting

- **[Documentation/CLION_SETUP.md](Documentation/CLION_SETUP.md)** - CLion + PlatformIO setup guide
  - Complete IDE integration guide
  - PlatformIO installation and configuration
  - Building and uploading from CLion
  - Code completion and navigation
  - Multi-platform development workflow

- **[Documentation/README_PLATFORMIO.md](Documentation/README_PLATFORMIO.md)** - PlatformIO quick reference
  - Quick start commands
  - Project structure
  - Library dependencies

### Platform Selection

- **[PLATFORM_COMPARISON.md](PLATFORM_COMPARISON.md)** - Choose between ESP8266 and Pico W
  - Side-by-side feature comparison
  - Performance metrics
  - When to choose each platform
  - Upgrade recommendations

### Preset Profile System (JEDEC-Compliant)

- **[Documentation/PRESET_PROFILES_FEATURE.md](Documentation/PRESET_PROFILES_FEATURE.md)** - 🆕 Complete preset system guide
  - Save and load temperature profiles
  - 5 built-in JEDEC-compliant presets
  - Prevent component damage (melted headers, etc.)
  - Custom preset creation
  - API reference

- **[Documentation/REFLOW_TEMPERATURE_STANDARDS.md](Documentation/REFLOW_TEMPERATURE_STANDARDS.md)** - 🆕 Industry standards reference
  - JEDEC J-STD-020 compliance details
  - Temperature ranges by solder type (SAC305, Sn63/Pb37)
  - Plastic component ratings and limitations
  - Time at temperature constraints (30-60s max)
  - Safety margins and best practices
  - Troubleshooting thermal damage

- **[Documentation/PRESET_QUICK_START.md](Documentation/PRESET_QUICK_START.md)** - 🆕 Quick start for presets
  - Immediate solution for heat-sensitive components
  - Temperature recommendations by component type
  - Step-by-step preset usage
  - Pro tips and troubleshooting

### PID Tuning and Control

- **[Documentation/AUTO_TUNE_FEATURE.md](Documentation/AUTO_TUNE_FEATURE.md)** - Automatic PID tuning feature
  - One-click auto-tuning process
  - How the algorithm works
  - Step-by-step usage instructions
  - Console log functionality
  - API endpoints

- **[Documentation/PID_TUNING_GUIDE.md](Documentation/PID_TUNING_GUIDE.md)** - Complete PID tuning guide
  - Automatic tuning instructions
  - Manual tuning guide
  - Understanding PID parameters
  - Troubleshooting temperature control
  - Technical algorithm details

- **[Documentation/PID_AUTOTUNE_LIBRARY_GUIDE.md](Documentation/PID_AUTOTUNE_LIBRARY_GUIDE.md)** - PID AutoTune library integration
  - Using the PID_AutoTune library
  - Library API reference
  - Configuration examples
  - Multi-temperature tuning

### Hardware and Troubleshooting

- **[Documentation/HARDWARE_TROUBLESHOOTING.md](Documentation/HARDWARE_TROUBLESHOOTING.md)** - Hardware troubleshooting guide
  - Thermocouple issues
  - SSR problems
  - Wiring verification
  - Common hardware failures

- **[Documentation/GPIO_PIN_MAPPING.md](Documentation/GPIO_PIN_MAPPING.md)** - Pin mapping reference
  - Pico W GPIO assignments
  - ESP8266 pin mapping
  - SPI bus configuration
  - Pin change history

- **[Documentation/LED_STATUS_GUIDE.md](Documentation/LED_STATUS_GUIDE.md)** - LED status indicators
  - What each LED pattern means
  - Troubleshooting via LED
  - Status code reference

### Web Interface and WiFi

- **[Documentation/WEB_INTERFACE_README.md](Documentation/WEB_INTERFACE_README.md)** - Web interface guide
  - Web interface usage
  - API documentation
  - WebSocket updates
  - Troubleshooting

- **[Documentation/CAPTIVE_PORTAL_TROUBLESHOOTING.md](Documentation/CAPTIVE_PORTAL_TROUBLESHOOTING.md)** - WiFi setup troubleshooting
  - Captive portal issues
  - Network connection problems
  - DNS configuration

### Architecture and Implementation

- **[Documentation/ARCHITECTURE_DIAGRAMS.md](Documentation/ARCHITECTURE_DIAGRAMS.md)** - System architecture
  - Block diagrams
  - Data flow
  - Component interactions
  - Dual-core architecture (Pico W)

- **[Documentation/IMPLEMENTATION_SUMMARY.md](Documentation/IMPLEMENTATION_SUMMARY.md)** - Implementation details
  - Code structure
  - Module organization
  - Design decisions

### Changelog and Version History

- **[Documentation/CHANGELOG.md](Documentation/CHANGELOG.md)** - Complete version history
  - All releases and changes
  - Feature additions
  - Bug fixes
  - Migration notes

## Features

### Common Features (Both Platforms)

✅ **Automatic PID Tuning** - One-click auto-tuning for optimal temperature control  
✅ **Web Interface** - Control from any device with a browser  
✅ **WiFi Connectivity** - Access Point or connect to existing network  
✅ **Real-Time Monitoring** - Live temperature updates  
✅ **Console Logging** - Real-time system messages and tuning progress  
✅ **PID Control** - Precise temperature management  
🆕 **Preset Profiles** - Save and load temperature profiles for different board types (prevents component damage!)

### Pico W Exclusive Features

⚡ **Dual-Core Architecture** - Dedicated core for thermal control  
⚡ **10x Faster PID Loop** - 10 Hz vs 1 Hz on ESP8266  
⚡ **Separate SPI Buses** - Independent thermocouple and display interfaces  
⚡ **Better Performance** - 133 MHz dual-core processor  
⚡ **More Memory** - 264 KB RAM vs 80 KB  
⚡ **Future-Ready** - Reserved pins for LCD display on SPI1
✅ **Temperature Charts** - Visual monitoring with Chart.js  
✅ **SSR Control** - Direct solid state relay control  
✅ **Safety Features** - Emergency stop, auto-shutdown on errors  
✅ **Configurable Profiles** - Adjust temperatures and times  
✅ **EEPROM Storage** - Settings persist across reboots  
✅ **No Display Required** - Simplified hardware, no pin conflicts  

## Hardware Requirements

| Component | Part | Notes |
|-----------|------|-------|
| Microcontroller | NodeMCU 1.0 ESP12E | ESP8266-based |
| Thermocouple Amp | Adafruit MAX31855 | K-Type only |
| Thermocouple | K-Type | High-temp probe |
| SSR | 3.3V logic compatible | 25A recommended |

## Pin Connections

```
ESP8266 Pin  →  Connection
D5 (GPIO14)  →  MAX31855 CLK
D4 (GPIO2)   →  MAX31855 CS
D6 (GPIO12)  →  MAX31855 DO
D8 (GPIO15)  →  SSR Control (+)
GND          →  SSR Control (-)
3.3V         →  MAX31855 VCC
GND          →  MAX31855 GND
```

## Software Setup

### Install ESP8266 Support

1. Arduino IDE → File → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Tools → Board → Board Manager → Search "ESP8266" → Install

### Install Libraries

Via Arduino Library Manager (Sketch → Include Library → Manage Libraries):
- `Adafruit MAX31855 library`
- `PID` by Brett Beauregard
- `WebSocketsServer` by Markus Sattler
- `ArduinoJson` by Benoit Blanchon

### Board Configuration

```
Board: NodeMCU 1.0 (ESP-12E Module)
Upload Speed: 115200
CPU Frequency: 80 MHz
Flash Size: 4M (3M SPIFFS)
```

## Usage

### Connect to Web Interface

**Default (AP Mode)**:
1. ESP8266 creates WiFi network "ReflowOven"
2. Connect to it (password: reflow123)
3. Open browser to http://192.168.4.1

**Station Mode** (connect to existing WiFi):
1. Edit code: `bool useAPMode = false;`
2. Set your WiFi credentials
3. Check Serial Monitor for IP address

### Run Reflow

**First Time Setup** (Recommended):
1. **Auto-Tune PID** - Click "🔧 Auto-Tune PID" in PID Tuning section
2. **Wait 15-30 minutes** - Monitor console log for progress
3. **Apply Values** - Copy recommended Kp, Ki, Kd values and save

**Normal Operation**:
1. **Monitor** - Check current temperature reads correctly
2. **Configure** - Set preheat, soak, and reflow parameters
3. **Start** - Click "START REFLOW" button
4. **Watch** - Monitor temperature and state changes
5. **Complete** - Automatic cooldown when finished

### Auto-Tune PID (Recommended for First Use)

The controller includes automatic PID tuning to optimize temperature control:

1. Ensure oven is **empty** (no PCBs)
2. Navigate to **⚙️ PID Tuning** section
3. Click **"🔧 Auto-Tune PID"** button
4. Watch **Console Log** for real-time progress
5. Wait for completion (15-30 minutes)
6. Copy recommended values to PID inputs
7. Click **"Save PID Settings"**

See [Documentation/AUTO_TUNE_FEATURE.md](Documentation/AUTO_TUNE_FEATURE.md) for detailed instructions.

### Emergency Stop

Click "EMERGENCY STOP" button anytime to:
- Immediately turn OFF heater (SSR)
- Reset to IDLE state
- Require restart to reflow again

## Default Reflow Profile

Suitable for lead-free solder (SAC305):

| Stage | Temperature | Duration |
|-------|-------------|----------|
| Preheat | 150°C | 90 sec |
| Soak | 180°C | 90 sec |
| Reflow | 230°C | 40 sec |
| Cooldown | Auto | ~60 sec |

Adjust via web interface for different solder types.

## Safety Features

⚠️ **IMPORTANT SAFETY INFORMATION**

This controller manages high temperatures and AC power:
- Always supervise the reflow process
- Keep fire extinguisher nearby
- Ensure proper ventilation
- Test without PCBs first
- Verify SSR is properly rated for your heater

**Built-in Safety:**
- Thermocouple error detection (auto-shutdown after 3 failures)
- Emergency stop button
- SSR forced OFF during errors
- SSR forced OFF during cooldown
- GPIO15 (SSR pin) initialized LOW on boot

## Troubleshooting

### Cannot connect to WiFi
- Check SSID is "ReflowOven" in AP mode
- Verify 2.4GHz network (ESP8266 doesn't support 5GHz)
- Check Serial Monitor for connection status

### Thermocouple reads NaN
- Verify wiring: CLK→D5, CS→D4, DO→D6
- Check 3.3V and GND connections
- Test thermocouple with MAX31855 example sketch

### SSR doesn't switch
- Verify SSR accepts 3.3V logic input
- Check D8 connection
- Use multimeter to verify voltage at SSR input
- Ensure SSR is rated for heater voltage/current

### Web interface won't load
- Verify correct IP address
- Clear browser cache
- Check Serial Monitor for server startup messages

## File Structure

```
Solder-Reflow-Oven/
├── README.md (this file)
├── PLATFORM_COMPARISON.md (platform selection guide)
├── Documentation/ (all documentation files)
│   ├── START_HERE.md
│   ├── QUICK_REFERENCE.md
│   ├── PRESET_PROFILES_FEATURE.md
│   ├── REFLOW_TEMPERATURE_STANDARDS.md
│   ├── AUTO_TUNE_FEATURE.md
│   ├── PID_TUNING_GUIDE.md
│   ├── PLATFORMIO_SETUP.md
│   ├── CLION_SETUP.md
│   ├── WEB_INTERFACE_README.md
│   ├── HARDWARE_TROUBLESHOOTING.md
│   ├── CHANGELOG.md
│   └── ... (and more)
├── src/ (modular source code)
│   ├── main.cpp
│   ├── presets.cpp/h (preset profile system)
│   ├── pid_controller.cpp/h
│   ├── state_machine.cpp/h
│   ├── temperature.cpp/h
│   ├── web_server.cpp/h
│   └── wifi_setup.cpp/h
├── Electrical Design/
│   ├── SolderReflowOvenPicoW/ (Pico W version)
│   └── SolderReflowOven/ (ESP8266 version)
├── platformio.ini (PlatformIO configuration)
└── Mechanical Design/
```

## API Reference

The controller exposes a REST API:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Web interface |
| `/api/status` | GET | Current status JSON |
| `/api/config` | GET | Get configuration |
| `/api/config` | POST | Update configuration |
| `/api/start` | POST | Start reflow |
| `/api/stop` | POST | Emergency stop |
| `/api/data` | GET | Temperature log |

WebSocket on port 81 for real-time updates.

## Customization

### Change WiFi Settings
```cpp
const char* ssid = "YourNetworkName";
const char* password = "YourPassword";
bool useAPMode = false;  // false = Station mode
```

### Adjust PID Parameters
```cpp
double Kp = 2, Ki = 5, Kd = 1;
```

### Modify Reflow Profile
Edit via web interface or in code:
```cpp
double preheatTemp = 150;
double soakTemp = 180;
double reflowTemp = 230;
unsigned long preheatTime = 90000;  // milliseconds
```

## Migration History

1. **Original**: Arduino Uno + MAX31856 + TFT Display
2. **Phase 1**: Ported to ESP8266 + MAX31855 + TFT Display (pin conflicts)
3. **Phase 2**: Removed display, added web interface
4. **Phase 3**: Added Pico W support with dual-core architecture
5. **Phase 4**: Added JEDEC-compliant preset profile system

See [Documentation/ESP8266_SETUP.md](Documentation/ESP8266_SETUP.md) for historical migration details.

## Future Enhancements

Potential improvements:
- [x] ~~Temperature curve charting on web interface~~ (DONE)
- [x] ~~Multiple saved reflow profiles~~ (DONE - Preset system)
- [ ] Email/push notifications on completion
- [ ] MQTT integration
- [ ] Data export to CSV
- [ ] Authentication for web interface
- [ ] Preset import/export

## Contributing

Feel free to submit issues, feature requests, or pull requests!

## License

Open source - use and modify for your projects.

## Credits

- Original design adapted from various reflow oven projects
- Web interface migration and ESP8266 port completed for improved usability
- Pico W dual-core version with 10x faster PID control
- JEDEC J-STD-020 compliant preset system
- Uses Adafruit libraries for MAX31855 support
- PID library by Brett Beauregard

## Support

For help:
1. Check **[Documentation/START_HERE.md](Documentation/START_HERE.md)** for quick start
2. Review **[Documentation/QUICK_REFERENCE.md](Documentation/QUICK_REFERENCE.md)** for common tasks
3. Check Serial Monitor output for diagnostic messages
4. Review **[Documentation/HARDWARE_TROUBLESHOOTING.md](Documentation/HARDWARE_TROUBLESHOOTING.md)**
5. Verify wiring matches pin diagram
6. Open an issue on GitHub

---

**⚠️ Safety First**: Always follow proper safety procedures when working with high temperatures and AC power!
