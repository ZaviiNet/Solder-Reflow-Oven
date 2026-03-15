# PID Tuning Guide for Solder Reflow Oven

## Overview
This guide explains how to use the new PID tuning controls and temperature monitoring features added to the reflow oven controller.

## What Changed?

### Critical Fix: Temperature-Based State Transitions
**Problem**: The oven was transitioning from REFLOW to COOLDOWN after 40 seconds, even if it only reached 201°C instead of the target 230°C.

**Solution**: The state machine now waits for the temperature to reach the target (within 5°C) before starting the reflow timer. This ensures the full 40 seconds at reflow temperature.

**Example**:
- Old behavior: Enter REFLOW → Wait 40s → COOLDOWN (temp might only be 201°C)
- New behavior: Enter REFLOW → Wait until 225°C reached → Wait 40s at temp → COOLDOWN

## New Features

### 1. PID Tuning Controls
Located in the "⚙️ PID Tuning" section of the web interface.

**Parameters**:
- **Kp (Proportional)**: Controls immediate response to temperature error
  - Increase: Faster heating, more aggressive
  - Decrease: Slower heating, more gentle
  - Default: 2.0

- **Ki (Integral)**: Eliminates steady-state error over time
  - Increase: Faster correction of persistent error
  - Decrease: Less aggressive long-term correction
  - Default: 5.0

- **Kd (Derivative)**: Reduces overshoot by dampening rapid changes
  - Increase: Less overshoot, may slow response
  - Decrease: Faster response, may overshoot
  - Default: 1.0

**How to Tune**:
1. Start with default values (Kp=2, Ki=5, Kd=1)
2. Run a test reflow cycle and observe the temperature chart
3. If heating is too slow, increase Kp (try 3.0 or 4.0)
4. If temperature overshoots target, increase Kd (try 2.0)
5. If temperature settles below target, increase Ki (try 7.0)
6. Click "Save PID Settings" to persist changes to EEPROM

**Tips**:
- Make one change at a time
- Small adjustments (0.5-1.0) are usually sufficient
- Monitor via the temperature chart to see effects
- Settings are saved automatically and persist across reboots

### 2. Real-Time Temperature Chart
Located in the "📊 Temperature Chart" section.

**Features**:
- Red line: Actual temperature
- Blue dashed line: Target setpoint
- X-axis: Time in seconds
- Y-axis: Temperature in °C
- Updates every second during reflow
- Maintains last 300 data points (5 minutes)
- Automatically clears when returning to IDLE

**How to Use**:
1. Start a reflow cycle
2. Watch the temperature chart update in real-time
3. Red line should closely follow blue line for optimal PID tuning
4. Use chart to diagnose heating issues:
   - Slow rise: Increase Kp
   - Overshoot: Increase Kd
   - Oscillation: Decrease Kp, increase Kd
   - Steady error: Increase Ki

### 3. EEPROM Storage
All settings now persist across power cycles.

**What's Saved**:
- Reflow profile (preheat/soak/reflow temperatures and times)
- PID parameters (Kp, Ki, Kd)
- Cooldown temperature

**How It Works**:
- Automatically loads on startup
- Saves when you click "Save Configuration" or "Save PID Settings"
- Falls back to defaults if no valid data found
- Uses magic number verification to ensure data integrity

## Troubleshooting

### Problem: Not Reaching Reflow Temperature in Time
**Symptoms**: System shows "WARNING: REFLOW timeout" message
**Solutions**:
1. Increase Kp for more aggressive heating
2. Check heater power and connections
3. Reduce thermal mass in oven
4. Increase safety timeout (edit maxStateTimeout in code)

### Problem: Temperature Overshoots Target
**Symptoms**: Temperature goes above setpoint, then drops below
**Solutions**:
1. Increase Kd to dampen response
2. Decrease Kp for less aggressive heating
3. Ensure proper SSR and heater sizing

### Problem: Temperature Oscillates Around Setpoint
**Symptoms**: Temperature bounces above and below target
**Solutions**:
1. Decrease Kp
2. Increase Kd
3. Check for thermal coupling issues

### Problem: Chart Not Displaying
**Symptoms**: Temperature Chart section is blank
**Solutions**:
1. Ensure ESP8266 has internet access (Chart.js loads from CDN)
2. Check browser console for errors
3. Try refreshing the page

## Safety Features

### Maximum State Timeout
- Each heating stage has a 3-minute safety timeout
- Prevents infinite loops if temperature cannot be reached
- System will transition anyway after timeout with warning
- Ensures heater turns off even if stuck

### Emergency Stop
- Red "EMERGENCY STOP" button always available during reflow
- Immediately stops heating and transitions to safe state
- All settings preserved for next cycle

## Serial Monitor Output

New log messages help diagnose issues:
```
REFLOW temperature reached at 227.3°C - starting reflow timer
```

Or if timeout occurs:
```
WARNING: REFLOW timeout - temperature not reached!
```

## Best Practices

1. **Always Monitor First Runs**: Watch the temperature chart closely on first runs with new settings
2. **Document Your Settings**: Note PID values that work well for your specific oven
3. **Safety First**: Never leave oven unattended during reflow
4. **Test Empty**: Test new PID settings with empty oven before processing boards
5. **Incremental Changes**: Make small PID adjustments and test each change

## Default Values

For reference, the default values are:
- Kp: 2.0
- Ki: 5.0
- Kd: 1.0
- Temperature threshold: 5°C (temp must be within 5°C of target)
- Safety timeout: 180 seconds (3 minutes)

These work well for most setups but may need adjustment based on:
- Oven size and thermal mass
- Heater wattage
- Insulation quality
- Ambient temperature
