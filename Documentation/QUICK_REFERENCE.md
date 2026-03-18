# Quick Reference Card

## 🚀 Quick Commands

### PlatformIO Build Commands
```bash
# Build ESP8266
pio run -e nodemcuv2

# Build Pico W
pio run -e pico

# Clean build
pio run -t clean

# Upload to board
pio run -e nodemcuv2 -t upload

# Serial monitor
pio device monitor -b 115200

# Build and upload
pio run -e nodemcuv2 -t upload && pio device monitor
```

### CLion Integration
```bash
# Generate CLion helper files
pio init --ide clion

# Reload CMake project
File → Reload CMake Project (in CLion)

# Invalidate caches
File → Invalidate Caches / Restart (in CLion)
```

## 📁 Project Structure
```
Solder-Reflow-Oven-IO/
├── platformio.ini          # PlatformIO config (MAIN)
├── CMakeLists.txt          # CLion integration
├── .clang-format           # Code style
├── src/
│   └── main.cpp           # Main source
├── include/               # Headers
├── lib/                   # Custom libraries
└── docs/                  # (All .md files)
```

## 📖 Documentation Quick Links

### Essential Setup
- **CLION_SETUP.md** - Complete CLion setup guide
- **README_PLATFORMIO.md** - PlatformIO quick start
- **README.md** - Main documentation

### PID Tuning
- **AUTO_TUNE_FEATURE.md** - Using auto-tune feature
- **PID_TUNING_GUIDE.md** - Manual tuning guide
- **PID_AUTOTUNE_LIBRARY_GUIDE.md** - Using PID_AutoTune library

### Platform Info
- **PLATFORM_COMPARISON.md** - ESP8266 vs Pico W
- **PICO_W_MIGRATION_GUIDE.md** - Upgrading to Pico W
- **PROJECT_SETUP_SUMMARY.md** - This project's setup status

## ⚙️ Configuration Files

### platformio.ini - Key Sections
```ini
[platformio]
default_envs = nodemcuv2    # Default build target

[env:nodemcuv2]             # ESP8266 NodeMCU
platform = espressif8266
board = nodemcuv2

[env:pico]                   # Raspberry Pi Pico W
platform = raspberrypi
board = pico
```

### WiFi Configuration (in main.cpp)
```cpp
const char* ssid = "YourSSID";
const char* password = "YourPassword";
bool useAPMode = true;  // true=AP, false=Station
```

## 🔧 Troubleshooting

### Build Issues
```bash
# Clean and rebuild
pio run -t clean && pio run -e nodemcuv2

# Update libraries
pio lib update

# Check library versions
pio lib list

# Verbose build output
pio run -e nodemcuv2 -v
```

### CLion Issues
```bash
# Reload CMake
File → Reload CMake Project

# Regenerate CLion files
pio init --ide clion

# Clear caches
File → Invalidate Caches / Restart
```

### Upload Issues
```bash
# List available ports
pio device list

# Specify port manually
pio run -e nodemcuv2 -t upload --upload-port /dev/ttyUSB0

# Check permissions (Linux)
sudo usermod -a -G dialout $USER
# Then logout/login
```

## 🎯 Common Tasks

### Start New Reflow Cycle
1. Connect to WiFi (ReflowOven / http://192.168.4.1)
2. Check temperature reading
3. Configure profile if needed
4. Click "START REFLOW"
5. Monitor progress

### Auto-Tune PID
1. Navigate to PID Tuning section
2. Click "🔧 Auto-Tune PID"
3. Wait 15-30 minutes
4. Copy recommended values
5. Click "Save PID Settings"

### Update Firmware
```bash
# Edit code in CLion
# Build in terminal
pio run -e nodemcuv2

# Upload to board
pio run -e nodemcuv2 -t upload

# Monitor serial output
pio device monitor
```

## 📊 Pin Connections

### ESP8266 NodeMCU
```
D5 (GPIO14) → MAX31855 CLK
D4 (GPIO2)  → MAX31855 CS
D6 (GPIO12) → MAX31855 DO
D8 (GPIO15) → SSR Control
3.3V        → MAX31855 VCC
GND         → MAX31855 GND
```

### Raspberry Pi Pico W
```
GPIO 18 → MAX31855 CLK
GPIO 17 → MAX31855 CS
GPIO 16 → MAX31855 DO
GPIO 15 → SSR Control
3.3V    → MAX31855 VCC
GND     → MAX31855 GND
```

## 🌐 Web Interface

### Default Access
- **AP Mode**: http://192.168.4.1
- **Station Mode**: Check serial monitor for IP

### API Endpoints
- `GET /` - Web interface
- `GET /api/status` - Current status
- `GET /api/config` - Get configuration
- `POST /api/config` - Update configuration
- `POST /api/start` - Start reflow
- `POST /api/stop` - Emergency stop
- `POST /api/tune-pid` - Start/stop auto-tune
- `GET /api/console` - Console log
- `WebSocket :81` - Real-time updates

## 🔐 Default Settings

### WiFi
- SSID: `ReflowOven`
- Password: `reflow123`
- Mode: AP (Access Point)

### Reflow Profile (Lead-Free)
- Preheat: 150°C for 90s
- Soak: 180°C for 90s
- Reflow: 230°C for 40s
- Cooldown: Auto to 25°C

### PID Parameters (Default)
- Kp: 2.0
- Ki: 5.0
- Kd: 1.0

## 📦 Library Versions
```ini
PID @ 1.2.1
PID-AutoTune @ 1.0.0
Adafruit MAX31855 library @ 1.4.2
ArduinoJson @ 7.2.2
WebSockets @ 2.7.3
```

## ⚠️ Safety Checks

### Before First Use
- [ ] Thermocouple connected correctly
- [ ] SSR rated for heater voltage/current
- [ ] SSR accepts 3.3V logic
- [ ] Fire extinguisher nearby
- [ ] Proper ventilation
- [ ] Empty oven test run

### During Operation
- [ ] Monitor temperature constantly
- [ ] Watch for thermal runaway
- [ ] Keep emergency stop ready
- [ ] Never leave unattended

## 💡 Tips & Tricks

### Performance
- Run PID auto-tune after hardware changes
- Use faster PID loop on Pico W (10Hz vs 1Hz)
- Monitor SSR clicking frequency

### Development
- Use CLion for editing, terminal for building
- Commit changes to Git frequently
- Test on empty oven first
- Document custom profiles

### Debugging
- Check serial monitor (115200 baud)
- Watch console log during auto-tune
- Verify temperature readings before reflow
- Use temperature chart to spot issues

## 🆘 Emergency Procedures

### If Temperature Runs Away
1. Click "EMERGENCY STOP" immediately
2. Unplug oven from AC power
3. Let cool naturally
4. Check SSR, wiring, and PID settings

### If Thermocouple Fails
1. System auto-stops after 3 read errors
2. Check wiring connections
3. Test with known good thermocouple
4. Verify 3.3V power

### If WiFi Won't Connect
1. Try AP mode (`useAPMode = true`)
2. Check 2.4GHz network (no 5GHz)
3. Serial monitor shows connection status
4. Reset ESP8266/Pico

## 📞 Getting Help

### Documentation
1. Check relevant .md file in project root
2. Review serial monitor output
3. Check web interface console log

### Community
- GitHub Issues: [Your repo URL]
- Project documentation: See README.md
- Hardware: Check WEB_INTERFACE_README.md

### Reporting Issues
Include:
- Platform: Raspberry Pi Pico W
- PlatformIO version: `pio --version`
- Library versions: `pio lib list`
- Error messages from serial monitor
- Steps to reproduce

---

**Platform**: Raspberry Pi Pico W (RP2040)
**Last Updated**: March 17, 2026
**Project Version**: See CHANGELOG.md

