# PID Tuning Guide for Solder Reflow Oven

## Overview
This guide explains how to use the PID tuning controls, automatic PID tuning, and temperature monitoring features of the reflow oven controller.

## What Changed?

### Critical Fix: Temperature-Based State Transitions
**Problem**: The oven was transitioning from REFLOW to COOLDOWN after 40 seconds, even if it only reached 201°C instead of the target 230°C.

**Solution**: The state machine now waits for the temperature to reach the target (within 5°C) before starting the reflow timer. This ensures the full 40 seconds at reflow temperature.

**Example**:
- Old behavior: Enter REFLOW → Wait 40s → COOLDOWN (temp might only be 201°C)
- New behavior: Enter REFLOW → Wait until 225°C reached → Wait 40s at temp → COOLDOWN

## New Features

### 1. Automatic PID Tuning (NEW!)
The controller now includes an **automatic PID tuning** feature that determines optimal PID values for your specific oven setup.

**What It Does**:
- Automatically tests your oven at three different temperatures (100°C, 150°C, 200°C)
- Creates controlled temperature oscillations by switching the heater on/off
- Measures how your oven responds (oscillation period and amplitude)
- Calculates optimal PID parameters using the Ziegler-Nichols method
- Displays results in a console log with recommended values

**How to Use**:
1. Ensure the oven is **empty** (no PCBs or components inside)
2. Make sure you have **15-30 minutes** available (tuning takes time)
3. Click the **"🔧 Auto-Tune PID"** button in the PID Tuning section
4. Confirm the tuning start dialog
5. Watch the **Console Log** for real-time progress updates
6. Wait for tuning to complete (or click "⏹ Stop Auto-Tune" to cancel)
7. Review the recommended PID values in the console
8. **Copy the values** to the Kp, Ki, Kd input fields
9. Click **"Save PID Settings"** to apply and persist the new values

**Understanding the Console Output**:
```
=== PID Auto-Tuning Started ===
Step 1/6: Heating to 100C...
Target 100C reached. Starting oscillation test...
  Oscillation #1: Period=45.2s, Amplitude=8.3C
  Oscillation #2: Period=46.1s, Amplitude=7.9C
  Oscillation #3: Period=45.8s, Amplitude=8.1C
Oscillation test complete at 100C
...
=== Calculating PID Parameters ===
Average oscillation period: 45.70 seconds
Average amplitude: 8.10 C
Ultimate gain (Ku): 0.157
Ultimate period (Pu): 45.70s

=== Recommended PID Values (Ziegler-Nichols) ===
Kp = 0.094
Ki = 0.002
Kd = 0.539
```

**Important Notes**:
- Auto-tuning works best with an **empty oven** (no thermal mass from PCBs)
- The process cannot be started during an active reflow cycle
- You can stop tuning at any time by clicking the button again
- Recommended values are conservative - you may need to fine-tune further
- The console can be shown/hidden using the "Show/Hide Console" button

**Safety**:
- Each test step has a 10-minute timeout
- SSR turns off automatically if errors occur
- Emergency stop button works during tuning

### 2. PID Tuning Controls
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

**Manual Tuning Process**:
1. Start with default values (Kp=2, Ki=5, Kd=1) or auto-tuned values
2. Run a test reflow cycle and observe the temperature chart
3. If heating is too slow, increase Kp (try 3.0 or 4.0)
4. If temperature overshoots target, increase Kd (try 2.0)
5. If temperature settles below target, increase Ki (try 7.0)
6. Click "Save PID Settings" to persist changes to EEPROM

**Tips**:
- **Try auto-tuning first** - it provides a good starting point
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

### Problem: Auto-Tuning Fails or Times Out
**Symptoms**: Tuning stops with "Not enough oscillation data" or timeout message
**Solutions**:
1. Ensure oven is completely empty (no PCBs or thermal mass)
2. Check heater power - if underpowered, oscillations may not be detectable
3. Verify SSR is switching properly (check SSR status indicator)
4. Increase heater power or reduce oven thermal mass
5. Try manual tuning instead

### Problem: Auto-Tuned Values Don't Work Well
**Symptoms**: Temperature control is unstable or sluggish with auto-tuned values
**Solutions**:
1. Auto-tuning provides conservative values as a starting point
2. Increase Kp by 20-50% for faster response
3. Adjust Ki if steady-state error exists
4. Fine-tune using manual tuning guidelines above
5. Re-run auto-tuning with different conditions (empty vs. with PCB)

### Problem: Console Log Not Updating
**Symptoms**: Console shows old messages or doesn't update during tuning
**Solutions**:
1. Click "Show/Hide Console" to toggle visibility
2. Refresh the web page
3. Check WebSocket connection in browser console
4. Wait a few seconds - console polls every 2 seconds

### Problem: Not Reaching Reflow Temperature in Time
**Symptoms**: System shows "WARNING: REFLOW timeout" message
**Solutions**:
1. Run auto-tuning to get optimal PID values
2. Increase Kp for more aggressive heating
3. Check heater power and connections
4. Reduce thermal mass in oven
5. Increase safety timeout (edit maxStateTimeout in code)

### Problem: Temperature Overshoots Target
**Symptoms**: Temperature goes above setpoint, then drops below
**Solutions**:
1. Increase Kd to dampen response
2. Decrease Kp for less aggressive heating
3. Run auto-tuning for better values
4. Ensure proper SSR and heater sizing

### Problem: Temperature Oscillates Around Setpoint
**Symptoms**: Temperature bounces above and below target
**Solutions**:
1. Decrease Kp
2. Increase Kd
3. Run auto-tuning for optimized values
4. Check for thermal coupling issues

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

1. **Use Auto-Tuning First**: Run auto-tuning before your first reflow to get optimized values for your oven
2. **Always Monitor First Runs**: Watch the temperature chart closely on first runs with new settings
3. **Document Your Settings**: Note PID values that work well for your specific oven
4. **Safety First**: Never leave oven unattended during reflow or auto-tuning
5. **Test Empty**: Test new PID settings with empty oven before processing boards
6. **Incremental Changes**: Make small PID adjustments and test each change

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

## Technical Details: Auto-Tuning Algorithm

For those interested in how the automatic tuning works:

### Method: Relay-Based Auto-Tuning (Åström-Hägglund Method)

The controller uses a relay (on/off) feedback method, which is particularly well-suited for thermal systems like reflow ovens.

**Process**:
1. **Heat to Target**: Brings oven to test temperature (100°C, 150°C, or 200°C)
2. **Create Oscillations**: Switches heater on when temp drops below target-5°C, off when above target+5°C
3. **Measure Response**: Records oscillation period (time between peaks) and amplitude (peak-to-valley difference)
4. **Repeat**: Collects at least 3 oscillations per temperature for accuracy
5. **Calculate Parameters**: Uses averaged measurements to determine system characteristics

**Calculations**:

The relay method estimates two critical system parameters:
- **Ultimate Gain (Ku)**: Maximum proportional gain before instability
  - Formula: `Ku = (4 × relay_amplitude) / (π × oscillation_amplitude)`
  - For our system: relay_amplitude = 1.0 (full on/off)

- **Ultimate Period (Pu)**: Oscillation period at the stability limit
  - Measured directly from oscillations

**Ziegler-Nichols Tuning Rules**:

From Ku and Pu, PID parameters are calculated using the classic Ziegler-Nichols method:
- **Kp** = 0.6 × Ku (proportional gain)
- **Ki** = 1.2 × Ku / Pu (integral gain, equivalent to Ti = Pu/2)
- **Kd** = 0.075 × Ku × Pu (derivative gain, equivalent to Td = Pu/8)

These formulas provide conservative tuning that prioritizes stability over aggressive response.

**Why Multiple Temperatures?**

Testing at different temperatures (100°C, 150°C, 200°C) helps account for non-linear behavior:
- Heat loss increases with temperature
- SSR and heater characteristics may vary with temperature
- Thermocouple response time can change

The algorithm averages results across all temperatures for robust PID values.

**Advantages of This Method**:
- Works without a mathematical model of the system
- Robust to noise and disturbances
- Simple to implement on microcontrollers
- Well-suited for systems with lag (like thermal processes)

**Limitations**:
- Takes time (15-30 minutes)
- Requires system to be operating (heater must work)
- Results are conservative (may need fine-tuning for optimal performance)
- Best with empty oven (thermal mass affects results)

### References
- Åström, K. J., & Hägglund, T. (1984). "Automatic tuning of simple regulators with specifications on phase and amplitude margins"
- Ziegler, J. G., & Nichols, N. B. (1942). "Optimum settings for automatic controllers"
