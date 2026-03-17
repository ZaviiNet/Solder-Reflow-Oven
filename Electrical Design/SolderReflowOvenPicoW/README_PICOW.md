# Solder Reflow Oven Controller - Raspberry Pi Pico W Edition

## Overview

This is an upgraded version of the Solder Reflow Oven Controller, migrated from ESP8266 to **Raspberry Pi Pico W** to leverage its dual-core RP2040 processor for improved performance and faster PID response.

### Key Improvements over ESP8266 Version

1. **Dual-Core Architecture**
   - **Core 0**: Handles WiFi, Web Server, and Web Interface
   - **Core 1**: Dedicated to temperature monitoring and PID control
   - Result: **10x faster PID loop** (10 Hz vs 1 Hz on ESP8266)

2. **Separate SPI Buses**
   - **SPI0**: Used for MAX31855 thermocouple (Core 1)
   - **SPI1**: Reserved for future display expansion (Core 0)
   - No bus contention between thermal control and UI

3. **Better Performance**
   - 133 MHz dual-core ARM Cortex-M0+ processor
   - 264 KB RAM (vs ~80 KB on ESP8266)
   - More responsive temperature control
   - Faster web interface

4. **Thread-Safe Communication**
   - Mutex-protected shared memory between cores
   - Reliable data exchange without race conditions

## Hardware Requirements

### Components

| Component | Specification | Notes |
|-----------|---------------|-------|
| Microcontroller | Raspberry Pi Pico W | Must be the "W" version with WiFi |
| Thermocouple Amplifier | Adafruit MAX31855 | K-Type thermocouple interface |
| Thermocouple | K-Type probe | Temperature range: -200°C to 1350°C |
| Solid State Relay | 3.3V logic compatible | For heater control |
| Power Supply | 5V USB or 3.3V | Micro USB for Pico W |

### Pin Connections

#### SPI0 - Thermocouple (Core 1)
```
Pico W Pin    →  MAX31855 Pin
─────────────────────────────
GPIO 18 (SPI0 SCK)   →  CLK
GPIO 16 (SPI0 RX)    →  DO (MISO)
GPIO 17              →  CS
3.3V                 →  VCC
GND                  →  GND
```

#### SPI1 - Reserved for Future Display (Core 0)
```
Pico W Pin    →  Future Display
────────────────────────────────
GPIO 10 (SPI1 SCK)   →  CLK
GPIO 11 (SPI1 TX)    →  MOSI
GPIO 12 (SPI1 RX)    →  MISO
GPIO 13              →  CS
```

#### Other GPIO
```
Pico W Pin    →  Component
───────────────────────────
GPIO 15       →  SSR Control (3.3V logic)
GPIO 25       →  Onboard LED (status)
```

### Wiring Diagram

```
                    ┌─────────────────────┐
                    │  Raspberry Pi       │
                    │  Pico W             │
                    │                     │
                    │  GPIO 18 ──────────┼──→ MAX31855 CLK
                    │  GPIO 16 ──────────┼──→ MAX31855 DO
                    │  GPIO 17 ──────────┼──→ MAX31855 CS
                    │                     │
                    │  GPIO 15 ──────────┼──→ SSR Control
                    │                     │
                    │  3.3V ─────────────┼──→ MAX31855 VCC
                    │  GND ──────────────┼──→ GND (Common)
                    │                     │
                    └─────────────────────┘
                             │
                             │ WiFi
                             ↓
                    Your Network / AP Mode
```

## Software Setup

### Arduino IDE Setup

1. **Install Arduino IDE 2.x** (or 1.8.19+)

2. **Install Pico Board Support**
   - Open **File → Preferences**
   - Add to "Additional Boards Manager URLs":
     ```
     https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
     ```
   - Go to **Tools → Board → Boards Manager**
   - Search for "pico"
   - Install **"Raspberry Pi Pico/RP2040"** by Earle F. Philhower, III

3. **Install Required Libraries**
   
   Go to **Tools → Manage Libraries** and install:
   - `PID` by Brett Beauregard
   - `Adafruit MAX31855 library` by Adafruit
   - `ArduinoJson` by Benoit Blanchon (v6.x)

4. **Select Board**
   - **Tools → Board → Raspberry Pi Pico/RP2040 → Raspberry Pi Pico W**
   - **Tools → Flash Size → 2MB (Sketch: 1MB, FS: 1MB)**
   - **Tools → CPU Speed → 133 MHz**
   - **Tools → Port → Select your Pico W**

5. **WiFi Configuration**
   
   Edit these lines in the sketch (around line 95):
   ```cpp
   const char* ssid = "ReflowOven";      // AP mode SSID or WiFi network
   const char* password = "reflow123";    // Password (min 8 chars)
   bool useAPMode = true;                 // false = connect to existing WiFi
   ```

6. **Upload**
   - Hold **BOOTSEL** button on Pico W
   - Connect USB cable
   - Release **BOOTSEL** (Pico W appears as USB drive)
   - Click **Upload** in Arduino IDE
   - Wait for compilation and upload

### PlatformIO Setup (Alternative)

If using PlatformIO, create `platformio.ini`:

```ini
[env:pico_w]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico_w
framework = arduino
board_build.core = earlephilhower
lib_deps = 
    br3ttb/PID@^1.2.1
    adafruit/Adafruit MAX31855 library@^1.4.1
    bblanchon/ArduinoJson@^6.21.3
build_flags = 
    -DWIFI_SSID=\"ReflowOven\"
    -DWIFI_PASSWORD=\"reflow123\"
monitor_speed = 115200
```

## Usage

### First Boot

1. **Power up** the Pico W
2. **Serial Monitor** (115200 baud) will show:
   ```
   ===========================================
   Solder Reflow Oven Controller - Pico W
   Raspberry Pi Pico W Dual-Core Version
   ===========================================
   
   SSR pin initialized (OFF)
   Initializing SPI0 for MAX31855 thermocouple...
   MAX31855 OK - Current temp: 25.3 °C
   PID controller initialized
   Configuration loaded successfully!
   PID Values - Kp: 2.00, Ki: 5.00, Kd: 1.00
   Starting WiFi Access Point...
   AP SSID: ReflowOven
   AP IP address: 192.168.4.1
   HTTP server started on port 80
   
   Core 0 (WiFi/Web) setup complete!
   Starting Core 1 (Thermal Control)...
   Both cores running!
   Core 0: Handling WiFi and Web Server
   Core 1: Handling Temperature and PID Control
   ```

3. **Connect to WiFi**
   - If in AP mode: Connect to "ReflowOven" network (password: reflow123)
   - If in Station mode: Pico W connects to your WiFi network

4. **Open Web Interface**
   - AP Mode: Navigate to `http://192.168.4.1`
   - Station Mode: Check serial monitor for IP address

### Web Interface

The web interface provides:

- **Real-time Status Display**
  - Current temperature
  - Target temperature
  - Current state (IDLE, PREHEAT, SOAK, REFLOW, COOLDOWN, COMPLETE)
  - Elapsed time

- **Reflow Control**
  - Start/Stop reflow cycle
  - Emergency stop

- **Configuration**
  - Reflow profile parameters (temperatures and times)
  - PID tuning parameters (Kp, Ki, Kd)
  - Save to EEPROM

- **PID Auto-Tuning**
  - Automatic PID parameter calculation
  - Console log display

### Reflow Profile

Default profile for lead-free solder (SAC305):

| Phase | Temperature | Duration |
|-------|-------------|----------|
| Preheat | 150°C | 90 seconds |
| Soak | 180°C | 90 seconds |
| Reflow | 230°C | 40 seconds |
| Cooldown | <35°C | 60 seconds |

Adjust these values in the web interface for different solder types.

### PID Auto-Tuning

The auto-tuning feature uses the Åström-Hägglund relay method with Ziegler-Nichols calculations:

1. Click **"Auto-Tune PID"** button
2. System tests at 3 temperatures: 100°C, 150°C, 200°C
3. For each temperature:
   - Heats to target
   - Creates controlled oscillations
   - Measures oscillation period and amplitude
4. Calculates optimal Kp, Ki, Kd values
5. Automatically applies new PID parameters
6. **Save configuration** to persist values

Expected tuning time: 10-15 minutes

## Architecture Details

### Dual-Core Design

```
┌─────────────────────────────────────────────────────────┐
│                    Core 0 (WiFi/Web)                    │
│  - WiFi connectivity (AP or Station mode)               │
│  - Web server (port 80)                                 │
│  - HTTP API endpoints                                   │
│  - Web interface serving                                │
│  - Configuration management                             │
│  - Data logging                                         │
│  - Console output                                       │
└─────────────────────────────────────────────────────────┘
                          ↕
              (Mutex-protected shared memory)
                          ↕
┌─────────────────────────────────────────────────────────┐
│                 Core 1 (Thermal Control)                │
│  - Temperature reading (10 Hz loop)                     │
│  - PID computation                                      │
│  - SSR control                                          │
│  - State machine                                        │
│  - Safety monitoring                                    │
│  - PID auto-tuning                                      │
│  - Thermocouple error detection                         │
└─────────────────────────────────────────────────────────┘
```

### Communication Between Cores

Cores communicate via **mutex-protected shared memory**:

```cpp
struct SharedData {
  double currentTemp;              // From Core 1 to Core 0
  double currentSetpoint;          // From Core 1 to Core 0
  double currentOutput;            // From Core 1 to Core 0
  ReflowState state;               // From Core 1 to Core 0
  bool ssrState;                   // From Core 1 to Core 0
  unsigned long elapsedTime;       // From Core 1 to Core 0
  bool emergencyStopRequested;     // From Core 0 to Core 1
  bool startReflowRequested;       // From Core 0 to Core 1
  bool stopReflowRequested;        // From Core 0 to Core 1
  bool pidTuningRequested;         // From Core 0 to Core 1
  bool stopTuningRequested;        // From Core 0 to Core 1
  bool newDataAvailable;           // Flag for updates
} sharedData;
```

Access is always protected:
```cpp
mutex_enter_blocking(&core_mutex);
// Read or write shared data
mutex_exit(&core_mutex);
```

### Performance Improvements

| Metric | ESP8266 | Pico W | Improvement |
|--------|---------|--------|-------------|
| CPU Speed | 80 MHz (single) | 133 MHz (dual) | 1.66x per core |
| PID Loop Rate | 1 Hz | 10 Hz | **10x faster** |
| RAM | ~80 KB | 264 KB | 3.3x more |
| Thermal Response | Delayed | Real-time | Significantly better |
| Web + Control | Shared | Separate cores | No interference |

## API Reference

### GET /api/status
Returns current system status.

**Response:**
```json
{
  "temp": 145.3,
  "setpoint": 150.0,
  "state": "PREHEAT",
  "elapsed": 45000,
  "ssr": 1,
  "output": 0.85,
  "tuning": 0
}
```

### GET /api/config
Returns current configuration.

**Response:**
```json
{
  "preheatTemp": 150,
  "soakTemp": 180,
  "reflowTemp": 230,
  "cooldownTemp": 25,
  "preheatTime": 90000,
  "soakTime": 90000,
  "reflowTime": 40000,
  "cooldownTime": 60000,
  "Kp": 2.0,
  "Ki": 5.0,
  "Kd": 1.0
}
```

### POST /api/config
Updates configuration.

**Request Body:**
```json
{
  "preheatTemp": 150,
  "Kp": 2.5,
  "Ki": 6.0,
  "Kd": 1.2,
  "saveToEEPROM": true
}
```

### POST /api/start
Starts reflow cycle.

**Response:**
```json
{
  "status": "success",
  "message": "Reflow started"
}
```

### POST /api/stop
Stops reflow cycle.

**Response:**
```json
{
  "status": "success",
  "message": "Reflow stopped"
}
```

### GET /api/data
Returns temperature log data.

**Response:**
```json
[
  {"time": 0, "temp": 25.3, "setpoint": 150, "state": 1},
  {"time": 1000, "temp": 26.1, "setpoint": 150, "state": 1},
  ...
]
```

### POST /api/tune-pid
Starts or stops PID auto-tuning.

**Request Body:**
```json
{
  "action": "start"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "PID tuning started"
}
```

### GET /api/console
Returns console log messages.

**Response:**
```json
[
  "=== PID Auto-Tuning Started ===",
  "Step 1/6: Heating to 100C...",
  "Target 100C reached. Starting oscillation test..."
]
```

## Safety Features

1. **Thermocouple Error Detection**
   - Monitors for NaN readings
   - 3 consecutive errors trigger emergency stop
   - Immediate SSR shutdown

2. **Emergency Stop**
   - Instantly turns off SSR
   - Sets state to ERROR
   - Prevents further heating

3. **State Timeouts**
   - Maximum 3 minutes per reflow phase
   - Prevents runaway heating
   - Automatic advancement or error

4. **Cooldown Enforcement**
   - SSR forced OFF during cooldown
   - Cannot be overridden

5. **Temperature Limits**
   - Software-enforced maximum temperatures
   - Configurable per phase

## Troubleshooting

### Issue: Thermocouple reads NaN

**Causes:**
- Loose wiring
- Wrong SPI pins
- Faulty MAX31855 board
- Faulty thermocouple

**Solutions:**
1. Check pin connections (GPIO 18, 16, 17)
2. Verify 3.3V power to MAX31855
3. Test thermocouple continuity
4. Check for cold solder joints
5. Try different SPI speed (in code)

### Issue: WiFi won't connect

**Solutions:**
1. Check SSID and password in code
2. Ensure WiFi network is 2.4 GHz (not 5 GHz)
3. Try AP mode instead
4. Check antenna connection on Pico W
5. Verify Pico W firmware is up to date

### Issue: SSR doesn't activate

**Solutions:**
1. Check GPIO 15 connection
2. Verify SSR trigger voltage (should be 3.3V)
3. Test SSR with multimeter
4. Check for loose wiring
5. Verify SSR orientation (+ and -)

### Issue: PID tuning fails

**Solutions:**
1. Increase tuning timeout
2. Check heater power (may be too weak)
3. Ensure system can reach target temperatures
4. Verify good thermal coupling
5. Check for thermal mass issues

### Issue: Web interface slow

**Solutions:**
1. Reduce number of clients
2. Check WiFi signal strength
3. Use wired connection if possible
4. Clear browser cache
5. Try different browser

## Advanced Configuration

### Changing SPI Speed

Edit in code (line ~412):
```cpp
SPI0_Thermo.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
```

Default: 4 MHz (safe for MAX31855)

### Adjusting PID Loop Rate

Edit in `runThermalControl()` (line ~444):
```cpp
if (currentTime - lastLoop >= 100) {  // 10 Hz (100ms)
```

Change `100` to desired interval in milliseconds:
- `50` = 20 Hz (faster)
- `200` = 5 Hz (slower)
- `100` = 10 Hz (default, recommended)

### Custom Reflow Profiles

Create presets in code or use web interface to save custom profiles.

Example for leaded solder (Sn63/Pb37):
```cpp
double preheatTemp = 120;
double soakTemp = 150;
double reflowTemp = 210;
double cooldownTemp = 25;
```

## Migration Guide (from ESP8266)

If you're upgrading from the ESP8266 version:

### Hardware Changes

| ESP8266 | Pico W | Notes |
|---------|--------|-------|
| D5 (GPIO14) | GPIO 18 | MAX31855 CLK |
| D4 (GPIO2) | GPIO 17 | MAX31855 CS |
| D6 (GPIO12) | GPIO 16 | MAX31855 DO |
| D8 (GPIO15) | GPIO 15 | SSR Control |
| 3.3V | 3.3V | Power |
| GND | GND | Ground |

### Software Changes

1. **Libraries**: Same libraries work (PID, ArduinoJson)
2. **WiFi**: Uses different WiFi library (built into Pico core)
3. **WebSocket**: Currently uses HTTP polling (can add WebSocket later)
4. **EEPROM**: Uses Pico's flash storage
5. **Performance**: 10x faster PID loop on dedicated core

### Configuration Transfer

Your existing configuration should be re-entered via web interface, as EEPROM format differs between platforms.

## Future Enhancements

Planned additions:

- [ ] WebSocket support for even faster updates
- [ ] LCD display on SPI1 (already wired and ready)
- [ ] Graph display on LCD
- [ ] SD card logging
- [ ] Multiple reflow profile storage
- [ ] OTA firmware updates
- [ ] Mobile app
- [ ] Bluetooth control
- [ ] Thermocouple linearization
- [ ] Multi-zone heating control

## Credits

- Original ESP8266 version: ZaviiNet
- Pico W port and dual-core architecture: Migration 2026
- PID Library: Brett Beauregard
- MAX31855 Library: Adafruit
- ArduinoJson: Benoit Blanchon

## License

Same license as original project. See main repository LICENSE file.

## Support

For issues, questions, or contributions, please visit the main repository:
https://github.com/ZaviiNet/Solder-Reflow-Oven

