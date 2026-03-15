# Changelog

## [Unreleased] - Enhanced PID Control and Monitoring

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
