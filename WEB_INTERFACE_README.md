# ESP8266 Web Interface - Solder Reflow Oven Controller

## Overview

This version of the Solder Reflow Oven Controller uses a **web-based interface** instead of a physical display, making it perfect for the ESP8266 platform and eliminating all the pin conflict issues with touchscreens.

## Features

✅ **Web-Based Control** - Access from any device with a browser (phone, tablet, computer)
✅ **Real-Time Monitoring** - WebSocket connection for live temperature updates
✅ **WiFi Connectivity** - Access Point or Station mode
✅ **PID Temperature Control** - Precise reflow profile execution
✅ **Emergency Stop** - Instant shutdown capability
✅ **Configurable Profiles** - Adjust preheat, soak, and reflow parameters
✅ **SSR Control** - Direct control of solid state relay for heater
✅ **Safety Features** - Automatic shutdown on thermocouple failure

## Hardware Requirements

| Component | Specification | Example |
|-----------|---------------|---------|
| Microcontroller | NodeMCU 1.0 ESP12E (ESP8266) | ESP-12E Module |
| Thermocouple Amp | Adafruit MAX31855 | K-Type Thermocouple Amplifier |
| Thermocouple | K-Type | High temperature K-Type probe |
| SSR | Solid State Relay (3.3V logic compatible) | 25A SSR, 3-32V DC control |
| Power Supply | 3.3V for ESP8266 | USB or dedicated 3.3V supply |

## Pin Connections

```
NodeMCU ESP8266          MAX31855          
┌─────────────┐         ┌─────────┐        
│             │         │         │        
│ D5 (GPIO14) ├────────►│ CLK     │        
│ D4 (GPIO2)  ├────────►│ CS      │        
│ D6 (GPIO12) │◄────────┤ DO      │        
│ 3V3         ├────────►│ VCC     │        
│ GND         ├────────►│ GND     │        
└─────────────┘         └─────────┘        

NodeMCU ESP8266          SSR
┌─────────────┐         ┌─────────┐
│             │         │         │
│ D8 (GPIO15) ├────────►│ Input+  │
│ GND         ├────────►│ Input-  │
└─────────────┘         └─────────┘
```

### Pin Summary

| Pin | GPIO | Function | Notes |
|-----|------|----------|-------|
| D5  | GPIO14 | MAX31855 CLK | SPI Clock |
| D4  | GPIO2  | MAX31855 CS | Chip Select |
| D6  | GPIO12 | MAX31855 DO | Data Out (MISO) |
| D8  | GPIO15 | SSR Control | Must be LOW on boot |

## Software Setup

### Required Libraries

Install these via Arduino Library Manager:

1. **ESP8266 Board Support**
   - Add URL in File → Preferences → Additional Board Manager URLs:
   - `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   
2. **Libraries to Install:**
   - `Adafruit MAX31855 library` by Adafruit
   - `PID` by Brett Beauregard
   - `WebSocketsServer` by Markus Sattler
   - `ArduinoJson` by Benoit Blanchon

### Arduino IDE Configuration

```
Board: NodeMCU 1.0 (ESP-12E Module)
Upload Speed: 115200
CPU Frequency: 80 MHz (or 160 MHz)
Flash Size: 4M (3M SPIFFS)
```

## WiFi Configuration

The code supports two modes:

### Access Point Mode (Default)

```cpp
const char* ssid = "ReflowOven";
const char* password = "reflow123";
bool useAPMode = true;
```

1. ESP8266 creates its own WiFi network named "ReflowOven"
2. Connect your device to this network (password: reflow123)
3. Navigate to `http://192.168.4.1`

### Station Mode (Connect to Existing WiFi)

```cpp
const char* ssid = "YourWiFiName";
const char* password = "YourWiFiPassword";
bool useAPMode = false;
```

1. ESP8266 connects to your existing WiFi
2. Check Serial Monitor for IP address
3. Navigate to the IP address shown

## Using the Web Interface

### Step 1: Connect to WiFi

- **AP Mode**: Connect to "ReflowOven" network, go to http://192.168.4.1
- **Station Mode**: Find IP in Serial Monitor, navigate to it

### Step 2: Monitor Status

The main page displays:
- **Current Temperature**: Real-time reading from thermocouple
- **Target Temperature**: Current setpoint from PID controller
- **State**: Current reflow stage (IDLE, PREHEAT, SOAK, REFLOW, COOLDOWN, COMPLETE, ERROR)
- **Time Elapsed**: Time since reflow started
- **SSR Status**: Heater on/off indicator with visual LED

### Step 3: Configure Reflow Profile

Default profile (suitable for lead-free solder):
- **Preheat**: 150°C for 90 seconds
- **Soak**: 180°C for 90 seconds
- **Reflow**: 230°C for 40 seconds
- **Cooldown**: Automatic cooling

To modify:
1. Enter desired temperatures and times
2. Click "Save Configuration"
3. Settings persist until changed again

### Step 4: Start Reflow

1. Ensure thermocouple shows reasonable room temperature
2. Click "START REFLOW" button
3. Monitor temperature and state changes
4. SSR indicator shows when heater is active

### Step 5: Emergency Stop

- Click "EMERGENCY STOP" at any time to immediately shut down heater
- System will turn SSR OFF and reset to IDLE state

## API Endpoints

The controller provides a REST API:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Web interface (HTML) |
| `/api/status` | GET | Current status (JSON) |
| `/api/config` | GET | Get current configuration |
| `/api/config` | POST | Update configuration |
| `/api/start` | POST | Start reflow process |
| `/api/stop` | POST | Emergency stop |
| `/api/data` | GET | Get logged data points |

### WebSocket

Real-time updates on port 81:
```javascript
ws://[IP_ADDRESS]:81
```

Messages are JSON format:
```json
{
  "temp": 25.5,
  "setpoint": 150.0,
  "state": "PREHEAT",
  "elapsed": 45000,
  "ssr": 1,
  "output": 0.8
}
```

## SSR Control Logic

The Solid State Relay is controlled as follows:

### PID Control (Preheat, Soak, Reflow)
```cpp
if (Output > 0.5) {
  digitalWrite(SSR_PIN, HIGH);  // Turn heater ON
} else {
  digitalWrite(SSR_PIN, LOW);   // Turn heater OFF
}
```

### Forced OFF (Cooldown, Complete, Error)
```cpp
digitalWrite(SSR_PIN, LOW);  // Always OFF
```

### Safety
- SSR automatically turns OFF on thermocouple errors
- SSR turns OFF when emergency stop is pressed
- SSR turns OFF during cooldown phase
- SSR is initialized to LOW (OFF) on startup

## Reflow Process

### State Machine

```
IDLE → PREHEAT → SOAK → REFLOW → COOLDOWN → COMPLETE
          ↓         ↓       ↓         ↓
        ERROR ←  ERROR ← ERROR ←  ERROR
```

### Typical Timeline

| Stage | Target Temp | Duration | Purpose |
|-------|-------------|----------|---------|
| PREHEAT | 150°C | 90s | Gradually warm components |
| SOAK | 180°C | 90s | Activate flux, even heating |
| REFLOW | 230°C | 40s | Melt solder |
| COOLDOWN | <35°C | 60s | Solidify joints |

## Safety Features

1. **Thermocouple Error Detection**
   - Monitors for NaN (not a number) readings
   - After 3 consecutive errors, triggers emergency stop
   - Displays ERROR state

2. **Emergency Stop Button**
   - Immediately turns OFF SSR
   - Resets state machine
   - Requires confirmation to prevent accidental clicks

3. **Automatic Cooldown**
   - Forces SSR OFF during cooldown
   - Won't allow reheating until complete

4. **Serial Logging**
   - All temperature readings logged
   - State changes logged
   - SSR status logged

## Troubleshooting

### Cannot Connect to WiFi

**AP Mode:**
- Ensure WiFi network "ReflowOven" is visible
- Check password is exactly "reflow123"
- Try restarting ESP8266

**Station Mode:**
- Check SSID and password are correct
- Ensure WiFi network is 2.4GHz (ESP8266 doesn't support 5GHz)
- Check Serial Monitor for connection status

### Thermocouple Reads NaN or 0

- Verify wiring: CLK to D5, CS to D4, DO to D6
- Check 3.3V and GND connections
- Ensure thermocouple is properly connected to MAX31855
- Test with example sketch first

### SSR Not Switching

- Verify SSR supports 3.3V logic input (most modern SSRs do)
- Check wiring to D8 (GPIO15) and GND
- Use multimeter to check voltage on SSR input terminals
- Ensure SSR is rated for your heater's voltage and current

### Web Interface Not Loading

- Check IP address is correct
- Ensure device is connected to same network
- Try clearing browser cache
- Check Serial Monitor for server startup messages

### Reflow Won't Start

- Check current state (must be IDLE, COMPLETE, or ERROR)
- Verify thermocouple is reading valid temperature
- Check Serial Monitor for error messages

## Serial Monitor Output

Example normal operation:
```
=================================
Solder Reflow Oven Controller
ESP8266 + MAX31855 + Web Interface
=================================

SSR pin initialized (OFF)
Initializing MAX31855 thermocouple...
MAX31855 OK - Current temp: 24.3 °C
PID controller initialized
Starting WiFi Access Point...
AP SSID: ReflowOven
AP IP address: 192.168.4.1
Connect to this network and navigate to http://192.168.4.1
HTTP server started on port 80
WebSocket server started on port 81

=================================
Setup complete!
=================================

Starting reflow process...
State change: IDLE -> PREHEAT
PREHEAT | Time: 5s | Temp: 28.5°C | Setpoint: 150.0°C | Output: 0.85 | SSR: ON
PREHEAT | Time: 10s | Temp: 45.2°C | Setpoint: 150.0°C | Output: 0.92 | SSR: ON
...
```

## Customization

### Modify Reflow Profile

Edit these values in the code or via web interface:
```cpp
double preheatTemp = 150;
double soakTemp = 180;
double reflowTemp = 230;
double cooldownTemp = 25;
unsigned long preheatTime = 90000;    // milliseconds
unsigned long soakTime = 90000;
unsigned long reflowTime = 40000;
unsigned long cooldownTime = 60000;
```

### Tune PID Parameters

Adjust for your specific heater characteristics:
```cpp
double Kp = 2, Ki = 5, Kd = 1;
```

- **Kp**: Proportional gain (higher = more aggressive)
- **Ki**: Integral gain (eliminates steady-state error)
- **Kd**: Derivative gain (reduces overshoot)

### Change WiFi Credentials

```cpp
const char* ssid = "YourNetworkName";
const char* password = "YourPassword";
bool useAPMode = false;  // true for AP, false for Station
```

## Advanced Features

### Data Logging

The controller logs up to 500 data points during reflow:
- Access via `/api/data` endpoint
- Returns JSON array of time, temperature, and setpoint
- Could be used for plotting temperature curves

### Future Enhancements

Potential improvements:
- Temperature curve charting on web interface
- Save/load multiple reflow profiles
- Email or push notifications when complete
- MQTT integration for home automation
- Export data logs to CSV

## License

This project is open source. Modify and use as needed for your solder reflow oven projects!

## Safety Warning

⚠️ **WARNING**: This controller manages high temperatures and AC-powered heaters.
- Always supervise the reflow process
- Ensure proper ventilation
- Use appropriate fire safety measures
- Test thoroughly before reflowing actual PCBs
- Verify all electrical connections are secure
- Keep flammable materials away from oven

## Support

For issues or questions:
1. Check Serial Monitor output for error messages
2. Verify wiring matches pinout diagram
3. Test thermocouple with example sketch
4. Ensure all libraries are installed correctly
