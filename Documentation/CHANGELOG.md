# Changelog

## [2.2.0] - 2026-03-19 - Over-The-Air (OTA) Firmware Updates

### Added - Major Feature
- **🆕 OTA Update System**: Update firmware over WiFi — no USB cable required
  - **ArduinoOTA support**: Upload firmware directly from Arduino IDE 2.x or PlatformIO
    - Device advertises as `reflow-oven.local` via mDNS
    - PlatformIO command: `pio run -t upload --upload-port reflow-oven.local`
    - Optional password protection via `OTA_PASSWORD` in `src/config.h`
  - **Browser-based HTTP upload**: Upload a compiled `.bin` file directly from the web UI
    - New endpoint: `POST /api/ota/update` — accepts multipart firmware upload
    - Real-time upload progress bar in the web interface
    - Device reboots automatically after a successful update
  - **Firmware version display**: Current version shown in the web UI and `/api/status`
  - **New files**: `src/ota.h`, `src/ota.cpp`
  - **OTA only active in Station mode**: ArduinoOTA is skipped when in captive-portal AP mode

### Changed
- `/api/status` response now includes `firmware` (version string) and `otaHostname` fields
- `FIRMWARE_VERSION` constant added to `src/config.h` (set to `"2.2.0"`)
- `OTA_HOSTNAME` constant added to `src/config.h` (default `"reflow-oven"`)

## [2.1.1] - 2026-03-18 - Preset Loading Bug Fix

### Fixed
- **Preset loading UI bug**: Form fields now update immediately when a preset is selected
  - Fixed promise chain in `loadSelectedPreset()` JavaScript function
  - Form fields (temperature and time) now update correctly after preset load
  - Added detailed success alert showing loaded values for verification
  - Improved error handling and dropdown reset on cancel/error
  - See `PRESET_LOADING_FIX.md` for technical details

## [2.1.0] - 2026-03-18 - Preset Profile System (JEDEC-Compliant)

### Added - Major Feature
- **🆕 Preset Profile System**: Save and load temperature profiles for different board types
  - **5 built-in JEDEC-compliant presets**: Based on J-STD-020 industry standards
    - Lead-Free Standard (245°C) - JEDEC compliant, 15°C safety margin
    - Lead-Free Safe (235°C) - Conservative for standard plastic headers
    - Leaded Solder (220°C) - Center of 210-230°C industry range
    - Sensitive Components (210°C) - For non-rated or cheap plastics
    - High Mass Boards (250°C) - Maximum JEDEC lead-free peak
  - **Custom preset creation**: Save your current settings as named presets
  - **Prevent component damage**: Temperature presets based on component ratings
  - **Web UI integration**: Dropdown selector and save/load buttons
  - **Flash storage**: Presets persist across reboots in `/presets.json`
  - **REST API endpoints**: `/api/presets`, `/api/presets/load`, `/api/presets/save`, `/api/presets/delete`
  - **New files**: `src/presets.h`, `src/presets.cpp`

### Documentation Added
- **PRESET_PROFILES_FEATURE.md**: Complete preset system documentation
  - Overview of preset system and benefits with JEDEC standards
  - All default presets with technical specifications
  - Time at temperature constraints (30-60s maximum)
  - Component ratings and safety margins
  - Web interface usage guide
  - API reference with examples
  - Best practices and troubleshooting

- **REFLOW_TEMPERATURE_STANDARDS.md**: Industry standards technical reference
  - JEDEC J-STD-020 compliance details
  - Temperature ranges by solder type (SAC305, Sn63/Pb37)
  - Plastic component ratings (260°C MSL 3, 250°C standard, etc.)
  - Time at temperature constraints (30-60 seconds at peak)
  - Safety margins (15°C recommended)
  - Troubleshooting thermal damage (browning, deformation)

- **PRESET_QUICK_START.md**: Quick start guide for presets
  - Immediate solution for melted component issues
  - Temperature recommendations by component rating
  - Step-by-step preset usage
  - Visual examples and comparisons
  - Pro tips and troubleshooting

- **PRESET_JEDEC_UPDATE.md**: Summary of JEDEC compliance updates
  - Temperature changes and justification
  - Technical standards applied
  - Component rating analysis

- **PRESET_UI_GUIDE.md**: Visual UI guide for web interface
  - UI layout and controls
  - Usage workflows with screenshots
  - Before/after comparisons

- **PRESET_LOADING_FIX.md**: Bug fix documentation

### Changed
- Web interface: Added preset dropdown and save controls
- Updated README.md with preset feature highlights and standards reference
- Main firmware: Added preset initialization in setup()
- Preset temperatures updated to JEDEC J-STD-020 standards

### Fixed
- ArduinoJson deprecation warnings: Replaced `containsKey()` with recommended `is<T>()` method
- Preset loading: Form fields now update correctly when preset is selected

## [2.0.0] - 2026-03-17 - Raspberry Pi Pico W Dual-Core Edition

### Added - Major Platform Addition
- **🆕 Raspberry Pi Pico W Support**: Complete new firmware version for RP2040 dual-core platform
  - Dedicated dual-core architecture: Core 0 handles WiFi/Web, Core 1 handles thermal control
  - **10x faster PID loop**: 10 Hz (100ms) vs 1 Hz (1000ms) on ESP8266
  - Separate SPI buses: SPI0 for thermocouple, SPI1 reserved for future display
  - Mutex-protected shared memory for thread-safe core communication
  - Better temperature control with reduced overshoot and smoother curves
  - 264 KB RAM vs 80 KB on ESP8266 (3.3x more memory)
  - Location: `Electrical Design/SolderReflowOvenPicoW/`

### Documentation Added
- **PICO_W_MIGRATION_GUIDE.md**: Complete migration guide from ESP8266 to Pico W
  - Hardware migration steps with pin mapping
  - Software setup instructions for Arduino IDE
  - Performance comparison and benefits
  - Troubleshooting common issues
  - FAQ section

- **README_PICOW.md**: Comprehensive Pico W documentation
  - Dual-core architecture explanation
  - Detailed pin connections and wiring diagrams
  - Setup and installation guide
  - API reference
  - Advanced configuration options
  - Future enhancement roadmap

### Changed
- **Updated main README.md**: Now covers both ESP8266 and Pico W versions
  - Clear platform comparison
  - Quick start guides for both platforms
  - Organized documentation by platform

### Technical Details - Pico W Architecture

#### Dual-Core Design
- **Core 0 (WiFi/Web)**: Runs at normal priority, handles all network and web interface tasks
  - WiFi connectivity (AP or Station mode)
  - Web server on port 80
  - HTTP API endpoints
  - Web interface serving
  - Configuration management
  
- **Core 1 (Thermal Control)**: Dedicated high-priority core for time-critical tasks
  - Temperature reading at 10 Hz
  - PID computation at 10 Hz
  - SSR control
  - State machine management
  - Safety monitoring
  - PID auto-tuning

#### Inter-Core Communication
- Mutex-protected shared data structure
- Thread-safe variable access
- Request/response flags for commands
- No race conditions or data corruption

#### Performance Improvements
| Metric | ESP8266 | Pico W | Improvement |
|--------|---------|--------|-------------|
| CPU Speed | 80 MHz | 133 MHz × 2 | 3.3x total processing |
| PID Loop | 1 Hz | 10 Hz | 10x faster |
| RAM | ~80 KB | 264 KB | 3.3x more |
| Thermal Response | Delayed | Real-time | Significant |

#### Pin Mapping Changes
```
ESP8266 NodeMCU  →  Raspberry Pi Pico W
─────────────────────────────────────────
D5 (GPIO14)      →  GPIO 18 (SPI0 SCK)
D4 (GPIO2)       →  GPIO 17 (CS)
D6 (GPIO12)      →  GPIO 16 (SPI0 MISO)
D8 (GPIO15)      →  GPIO 15 (SSR)
```

#### Future-Ready Features
- SPI1 pins reserved for LCD display (GPIO 10-13)
- Can add local display without interfering with thermal control
- Expandable for multi-zone control
- OTA update capability ready

### Migration Path
- Existing ESP8266 users can upgrade with minimal changes
- Same component reuse: MAX31855, thermocouple, SSR
- Configuration must be manually re-entered (different EEPROM format)
- Recommend running PID auto-tune after migration
- Estimated migration time: 1-2 hours

### Compatibility
- **Requires**: Raspberry Pi Pico W (WiFi version)
- **Arduino IDE**: Requires RP2040 board support
- **Libraries**: Same PID, ArduinoJson, Adafruit MAX31855 libraries
- **WiFi**: 2.4 GHz only (not 5 GHz compatible)

### Known Limitations - Pico W Version
- WebSocket not yet implemented (uses HTTP polling)
- Slightly slower web interface than ESP8266 (WiFi stack difference)
- No backward compatibility with ESP8266 EEPROM data

### Backward Compatibility
- **ESP8266 version remains fully supported** in `Electrical Design/SolderReflowOven/`
- No breaking changes to ESP8266 version
- Both versions maintained in parallel

## [1.x] - Enhanced PID Control and Monitoring

### Added
- **PID Tuning Controls**: Web interface now includes adjustable PID parameters (Kp, Ki, Kd) with helpful descriptions
- **EEPROM Storage**: Configuration and PID parameters are now saved to EEPROM and persist across reboots
- **Real-time Temperature Graphing**: Live temperature chart shows current temperature and setpoint during reflow process
- **Temperature-Based State Transitions**: State machine now waits for temperature targets to be reached before transitioning
- **Enhanced Safety**: Maximum state timeout (3 minutes) prevents indefinite waiting if temperature targets aren't reached

### Changed
- **Fixed Critical Issue**: System now waits for temperature to reach 230°C before starting the 40-second reflow timer
  - Previous behavior: transitioned to cooldown after 40 seconds regardless of actual temperature
  - New behavior: waits for temperature to reach within 5°C of target, then runs for configured duration
- **Improved State Machine Logic**: All heating stages (PREHEAT, SOAK, REFLOW) now verify temperature targets are achieved
- **Configuration Management**: PID parameters are now included in configuration save/load operations

### Technical Details

#### State Machine Updates
The state machine now uses temperature-aware transitions:
- **PREHEAT**: Waits until temperature reaches 145°C+ (150°C - 5°C threshold) before completing
- **SOAK**: Waits until temperature reaches 175°C+ (180°C - 5°C threshold) before completing  
- **REFLOW**: Waits until temperature reaches 225°C+ (230°C - 5°C threshold) before starting timer
- All stages have a 3-minute safety timeout to prevent infinite loops

#### EEPROM Storage
- Configuration stored at EEPROM address 0
- Magic number (0xABCD) validates data integrity
- Stores: Profile temperatures/times + PID parameters (Kp, Ki, Kd)
- Auto-loads on startup, falls back to defaults if no valid data found

#### Web Interface Improvements
- New "Temperature Chart" card with real-time Chart.js graph (requires internet for CDN)
- New "PID Tuning" card with three adjustable parameters
- Help text explains what each PID parameter controls
- Chart automatically clears when returning to IDLE state
- Chart maintains last 300 data points (5 minutes at 1 sample/second)

### Migration Notes
- First boot after update will use default values
- Save configuration once to persist to EEPROM
- No changes to existing pin assignments or hardware requirements

### Dependencies
- Chart.js 4.4.0 (loaded via CDN from jsdelivr.net)
- All other dependencies remain unchanged

### Issue Resolution
This update addresses the problem where the system transitioned from REFLOW to COOLDOWN before reaching the target temperature of 230°C. The new temperature-aware state machine ensures that:

1. The target temperature is reached before timing begins
2. The system stays at reflow temperature for the full configured duration
3. Safety timeouts prevent system hangs if temperature cannot be reached
4. Real-time graphs allow operators to monitor and diagnose temperature performance
5. PID tuning can be adjusted without recompiling and reflashing the firmware
