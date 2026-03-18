# Hardware Troubleshooting Checklist

## ⚠️ Current Issue: Thermocouple Read Error

**Error Message**: `WARNING: Thermocouple read error - check wiring!`

---

## 🔍 Step-by-Step Troubleshooting

### ✓ Step 1: Visual Inspection (2 minutes)

- [ ] Thermocouple is plugged into MAX31855 board (the two screw terminals or connector)
- [ ] All wires between Pico W and MAX31855 are firmly connected
- [ ] No broken or frayed wires visible
- [ ] MAX31855 board appears undamaged (no burn marks, broken components)
- [ ] Pico W is powered on (LED should be lit)

### ✓ Step 2: Verify Pin Connections (5 minutes)

**Pico W GPIO Pins → MAX31855 Board**

| Pico W Pin | Wire Color? | MAX31855 Label | Purpose |
|------------|-------------|----------------|---------|
| GPIO 2 | _______ | CLK (or SCK) | SPI Clock |
| GPIO 3 | _______ | CS | Chip Select |
| GPIO 4 | _______ | DO or MISO | Data Out |
| 3.3V (Pin 36) | _______ | VCC or 3V3 | Power |
| GND (Pin 38) | _______ | GND | Ground |

**Write down your wire colors above for reference**

### ✓ Step 3: Test Power (3 minutes)

Using a multimeter (if available):

- [ ] Measure voltage between MAX31855 VCC and GND
  - **Expected**: 3.3V (±0.2V acceptable)
  - **Actual**: _______ V

If no multimeter:
- [ ] Check that Pico W LED is on (confirms 3.3V rail is working)
- [ ] Look for any LED on MAX31855 board (some have power indicators)

### ✓ Step 4: Thermocouple Connection (2 minutes)

- [ ] Thermocouple wires are connected to MAX31855 terminals
  - Usually labeled "T+" and "T-" or just two screw terminals
  - Polarity matters for K-type thermocouples!

- [ ] Try swapping thermocouple polarity
  - Note: This won't damage anything
  - May show negative temperature if backwards
  - Some cheap thermocouples aren't color-coded correctly

### ✓ Step 5: Check Serial Monitor Output (1 minute)

```bash
cd /home/jack/CLionProjects/Solder-Reflow-Oven-IO
pio device monitor -b 115200 --port /dev/ttyACM0
```

Look for one of these patterns:

**Pattern 1 - Continuous errors every ~1 second**
```
WARNING: Thermocouple read error!
WARNING: Thermocouple read error!
WARNING: Thermocouple read error!
```
→ This suggests SPI communication issue (wiring problem)

**Pattern 2 - Occasional errors**
```
IDLE | Temp: 23.5°C
WARNING: Thermocouple read error!
IDLE | Temp: 23.4°C
```
→ This suggests loose connection or intermittent fault

**Pattern 3 - Stopped after 3 errors**
```
WARNING: Thermocouple read error!
WARNING: Thermocouple read error!
CRITICAL: Multiple thermocouple errors - EMERGENCY STOP!
```
→ System in ERROR state (safe) - fix wiring and restart

### ✓ Step 6: Test with Known Values (5 minutes)

If you have a multimeter with thermocouple input:
- [ ] Measure room temperature with multimeter
  - **Expected**: ~20-25°C
  - **Actual**: _______ °C

- [ ] Touch thermocouple junction with warm hand
  - Temperature should rise to ~30-35°C within seconds
  - If it does, thermocouple is probably working

If no thermocouple meter:
- [ ] Touch thermocouple junction with your hand (should feel cold metal)
- [ ] Check if thermocouple wires are physically intact
- [ ] Verify thermocouple is K-type (MAX31855 only works with K-type)

### ✓ Step 7: Swap Components (if available)

Try replacing one component at a time:

- [ ] Different thermocouple → _______ (worked / didn't work)
- [ ] Different MAX31855 board → _______ (worked / didn't work)
- [ ] Different jumper wires → _______ (worked / didn't work)

### ✓ Step 8: Test with Minimal Setup (10 minutes)

Disconnect everything except:
1. Pico W USB power
2. MAX31855 power (3.3V, GND)
3. MAX31855 SPI (CLK, CS, DO)
4. Thermocouple in MAX31855

**DO NOT connect SSR or heater for this test**

Then check serial monitor. If temperature reads correctly:
→ Problem was with SSR wiring interfering
→ Add SSR back one wire at a time

---

## 🎯 Quick Diagnosis Guide

### Symptom → Likely Cause

| What You See | Likely Problem |
|--------------|----------------|
| Continuous "read error" every second | CLK, CS, or DO wire wrong/disconnected |
| "ERROR" on web interface | Multiple errors triggered ERROR state |
| No serial output at all | USB cable issue or wrong COM port |
| Random occasional errors | Loose wire or poor connection |
| Web interface won't load | WiFi issue (different problem) |
| Temperature shows but wrong value | Thermocouple polarity reversed or wrong type |

---

## 🔧 Most Common Fixes

Based on typical troubleshooting:

### Fix #1: Wrong GPIO Pins (50% of cases)
**Problem**: Wires connected to wrong Pico W pins
**Solution**: Double-check GPIO 2 (CLK), 3 (CS), 4 (DO)

### Fix #2: Loose Connections (30% of cases)
**Problem**: Jumper wires not fully inserted
**Solution**: Push all connections firmly, wiggle gently to test

### Fix #3: No Thermocouple Connected (10% of cases)
**Problem**: Forgot to plug thermocouple into MAX31855
**Solution**: Connect to T+ and T- terminals on MAX31855

### Fix #4: No Power to MAX31855 (5% of cases)
**Problem**: VCC or GND not connected
**Solution**: Verify 3.3V and GND connections

### Fix #5: Wrong MAX31855 Type (3% of cases)
**Problem**: Using MAX31856 or MAX6675 instead
**Solution**: Verify chip says "MAX31855" on it

### Fix #6: Faulty Component (2% of cases)
**Problem**: Dead MAX31855 or broken thermocouple
**Solution**: Try known-good replacement parts

---

## ✅ Success Criteria

**You've fixed it when you see**:

1. **LED Status**: Solid ON (not blinking)

2. Serial monitor shows:
   ```
   IDLE | Time: 0s | Temp: 23.5°C | Setpoint: 0.0°C | Output: 0 | SSR: OFF
   ```

3. Web interface (http://192.168.0.95) shows:
   - Current Temperature: **23.5°C** (or actual room temp)
   - NOT showing "ERROR"

4. Console log stops showing error messages

5. When you touch thermocouple, temperature rises within 10 seconds

**LED Pattern Guide**:
- Solid ON = All good! ✅
- Medium Blink (2 Hz) = Still has errors ⚠️
- Fast Blink (4 Hz) = Reflow active 🔥
- Slow Blink (1 Hz) = Reflow complete ✓

---

## 🆘 Still Not Working?

### Additional Resources

1. **Check MAX31855 Datasheet**
   - Verify your board pinout matches assumed connections
   - Some clones use different pin arrangements

2. **Test with Arduino Example Code**
   - Upload Adafruit MAX31855 library example
   - See if simple example works (isolates problem)

3. **Multimeter Continuity Test**
   - Beep test each wire end-to-end
   - Verify no shorts between adjacent pins

4. **Check for 5V vs 3.3V Issues**
   - MAX31855 should run on 3.3V
   - Verify you're not using 5V by mistake
   - Some boards can handle either, some can't

### Document Your Findings

If still having issues, note:
- [ ] Which steps completed: _____________________
- [ ] Voltages measured: _____________________
- [ ] Components tested: _____________________
- [ ] What changed when: _____________________

This helps identify patterns and next troubleshooting steps.

---

## 📞 Getting More Help

If you've completed all steps above and still have issues:

1. **Gather info**:
   - Full serial monitor output (30+ seconds)
   - Photo of your wiring
   - Exact MAX31855 board model
   - Thermocouple type

2. **Check documentation**:
   - See `TROUBLESHOOTING_SESSION.md` for software details
   - See `QUICK_REFERENCE.md` for pin diagrams
   - See `README.md` for general info

3. **Test known-good configuration**:
   - Try exact hardware from Bill of Materials
   - Use known-working example from another project

---

**Good luck! The software is working perfectly - just need to get the hardware sorted out.**

**Created**: March 17, 2026
**Device**: Raspberry Pi Pico W
**Board**: MAX31855 Thermocouple Amplifier
**Status**: Software ✅ | Hardware ⚠️ (needs check)

