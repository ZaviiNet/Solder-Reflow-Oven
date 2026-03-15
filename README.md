# Solder Reflow Oven Controller - ESP8266 Edition

This repository contains a complete reflow oven controller designed for the **NodeMCU 1.0 ESP12E (ESP8266)** microcontroller with a **web-based interface**.

## Quick Start

1. **Hardware**: NodeMCU ESP8266 + MAX31855 Thermocouple + SSR
2. **Software**: Arduino IDE with ESP8266 board support
3. **Setup**: Upload code, connect to "ReflowOven" WiFi
4. **Access**: Navigate to http://192.168.4.1
5. **Use**: Configure profile, click START REFLOW

## Documentation

- **[WEB_INTERFACE_README.md](WEB_INTERFACE_README.md)** - Complete guide for the web interface version
  - Pin connections and wiring diagrams
  - WiFi setup (AP and Station modes)
  - Web interface usage
  - SSR control implementation
  - API documentation
  - Troubleshooting

- **[ESP8266_SETUP.md](ESP8266_SETUP.md)** - Historical reference for ESP8266 + MAX31855 with display
  - Original migration from Arduino Uno
  - Pin mapping details
  - Library compatibility notes
  - Display version information (now replaced with web interface)

## Features

✅ **Web Interface** - Control from any device with a browser  
✅ **WiFi Connectivity** - Access Point or connect to existing network  
✅ **Real-Time Monitoring** - Live temperature updates via WebSocket  
✅ **PID Control** - Precise temperature management  
✅ **SSR Control** - Direct solid state relay control  
✅ **Safety Features** - Emergency stop, auto-shutdown on errors  
✅ **Configurable Profiles** - Adjust temperatures and times  
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

1. **Monitor** - Check current temperature reads correctly
2. **Configure** - Set preheat, soak, and reflow parameters
3. **Start** - Click "START REFLOW" button
4. **Watch** - Monitor temperature and state changes
5. **Complete** - Automatic cooldown when finished

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
├── WEB_INTERFACE_README.md (detailed web interface documentation)
├── ESP8266_SETUP.md (ESP8266 migration reference)
├── Electrical Design/
│   ├── SolderReflowOven/
│   │   ├── SolderReflowOven.ino (main web interface version)
│   │   └── SolderReflowOven_display_version.ino.bak (original display version backup)
│   └── ESP8266_MAX31855_Example/
│       └── ESP8266_MAX31855_Example.ino (thermocouple test sketch)
├── Bill of Materials (original BOM)
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
3. **Phase 2**: Removed display, added web interface (current version)

See ESP8266_SETUP.md for migration details.

## Future Enhancements

Potential improvements:
- [ ] Temperature curve charting on web interface
- [ ] Multiple saved reflow profiles
- [ ] Email/push notifications on completion
- [ ] MQTT integration
- [ ] Data export to CSV
- [ ] Authentication for web interface

## Contributing

Feel free to submit issues, feature requests, or pull requests!

## License

Open source - use and modify for your projects.

## Credits

- Original design adapted from various reflow oven projects
- Web interface migration and ESP8266 port completed for improved usability
- Uses Adafruit libraries for MAX31855 support
- PID library by Brett Beauregard

## Support

For help:
1. Check Serial Monitor output
2. Review WEB_INTERFACE_README.md troubleshooting section
3. Verify wiring matches pin diagram
4. Test thermocouple with example sketch
5. Open an issue on GitHub

---

**⚠️ Safety First**: Always follow proper safety procedures when working with high temperatures and AC power!
