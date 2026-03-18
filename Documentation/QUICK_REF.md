# Quick Reference Card

## File Structure at a Glance

```
src/
├── main.cpp (old)          → main_old.cpp (backup)
├── main_new.cpp            → main.cpp (active)
│
├── pins.h                  Pin definitions
├── config.h                Constants & structures
│
├── temperature.{h,cpp}     Thermocouple sensor
├── pid_controller.{h,cpp}  PID control + tuning
├── state_machine.{h,cpp}   Reflow state machine
├── led_control.{h,cpp}     LED indicators
├── storage.{h,cpp}         Flash storage (LittleFS)
├── wifi_setup.{h,cpp}      WiFi & captive portal
└── web_server.{h,cpp}      HTTP server & API
```

## Quick Commands

### Migration
```bash
./migrate_to_modular.sh
```

### Build & Upload
```bash
pio run                    # Build
pio run --target upload    # Upload
pio device monitor         # Monitor
```

### Rollback
```bash
cd src
mv main.cpp main_new.cpp
mv main_old.cpp main.cpp
```

## Module Quick Reference

| Need to... | Edit this file |
|------------|----------------|
| Change pin assignments | `pins.h` |
| Modify reflow stages | `state_machine.cpp` |
| Tune PID algorithm | `pid_controller.cpp` |
| Update web interface | `web_server.cpp` |
| Change WiFi behavior | `wifi_setup.cpp` |
| Adjust sensor settings | `temperature.cpp` |
| Modify LED patterns | `led_control.cpp` |
| Change storage format | `storage.cpp` |

## Web API Endpoints

```
GET  /                    Main page
GET  /wifi                WiFi setup
POST /wifi/save           Save WiFi credentials
GET  /api/status          Current status (JSON)
GET  /api/config          Get configuration
POST /api/config          Save configuration
POST /api/start           Start reflow
POST /api/stop            Emergency stop
POST /api/tune-pid        Start/stop auto-tune
```

---

**Keep this file handy for quick reference!** 📋

