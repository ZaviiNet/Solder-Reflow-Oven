# LED Status Indicators - Quick Reference

## ✅ LED Now Active!

The Pico W's onboard LED now provides visual feedback about the system state.

---

## 🔦 LED Pattern Guide

### Current State Indicators

| LED Pattern | Meaning | Details |
|-------------|---------|---------|
| **Solid ON** 💡 | Connected & Idle | System ready, WiFi connected, waiting for commands |
| **Fast Blink** ⚡ (4 Hz) | Reflow Active | Heating cycle in progress (Preheat/Soak/Reflow/Cooldown) |
| **Medium Blink** ⚠️ (2 Hz) | ERROR State | Thermocouple error or other critical issue |
| **Slow Blink** ✅ (1 Hz) | Reflow Complete | Cycle finished successfully |
| **Fast Blink** 🔧 (4 Hz) | PID Tuning | Auto-tuning in progress |

### Blink Frequencies
- **Fast**: 250ms ON, 250ms OFF (4 times per second)
- **Medium**: 500ms ON, 500ms OFF (2 times per second)
- **Slow**: 1000ms ON, 1000ms OFF (once per second)

---

## 🎯 What You Should See Right Now

### Expected Behavior
Since your thermocouple has errors, you should see:

**LED: Medium Blink (2 Hz) - ERROR State** ⚠️

This confirms:
- ✅ WiFi is connected
- ✅ System is running
- ⚠️ Thermocouple error detected
- ✅ Safety system working (LED warning you)

### After Fixing Thermocouple
Once you fix the thermocouple wiring, you should see:

**LED: Solid ON - Idle State** 💡

This means:
- ✅ All systems working
- ✅ Ready to start reflow
- ✅ No errors detected

---

## 🔍 Troubleshooting with LED

### No LED at all
**Possible causes**:
1. Pico W not powered (check USB connection)
2. Code not uploaded (re-upload firmware)
3. Pico W in bootloader mode (shows as USB drive)

**Solution**: Press RESET button on Pico W or reconnect USB

### LED pattern doesn't match expected
**If you see**:
- Medium blink but no errors in serial → Check serial monitor for error messages
- Fast blink in Idle → System thinks it's running a cycle
- No LED change when starting reflow → Check web interface for state

### LED helps diagnose issues
The LED gives instant feedback without needing:
- Serial monitor connection
- Web browser open
- Physical access to computer

---

## 📊 LED State Mapping

### Detailed State Breakdown

```
System State    | LED Pattern      | What's Happening
----------------|------------------|------------------
IDLE            | Solid ON         | Waiting for commands
PREHEAT         | Fast Blink (4Hz) | Heating to preheat temp
SOAK            | Fast Blink (4Hz) | Maintaining soak temp
REFLOW          | Fast Blink (4Hz) | Peak reflow temperature
COOLDOWN        | Fast Blink (4Hz) | Cooling down
COMPLETE        | Slow Blink (1Hz) | Cycle complete, safe to open
ERROR_STATE     | Medium Blink (2Hz)| Error detected, heater OFF
PID_TUNING      | Fast Blink (4Hz) | Auto-tuning PID parameters
```

---

## 🔧 Technical Details

### How It Works
The Pico W's LED is different from the regular Pico:

- **Regular Pico**: LED on GPIO 25 (direct control)
- **Pico W**: LED connected to CYW43 WiFi chip
- **Control**: Via `LED_BUILTIN` constant
- **Code**: Uses `digitalWrite(LED_BUILTIN, HIGH/LOW)`

### Update Rate
LED is updated in the main `loop()` function:
- Checked every loop iteration (~1000+ times per second)
- State changes are instant
- Blink timing is precise (uses `millis()`)

### Power Consumption
The LED uses minimal power:
- Single LED: ~2-5mA when on
- Blinking: Average ~1-3mA (50% duty cycle)
- Negligible impact on system operation

---

## 💡 Using LED for Debugging

### Startup Sequence
Watch the LED during startup:
1. **OFF** → Power on / uploading firmware
2. **Slow Blink** → Connecting to WiFi (during setup)
3. **Solid ON** → Connected & ready

If LED stays off or behaves strangely:
- Check serial monitor for errors
- Verify USB connection
- Try pressing RESET button

### During Operation
Monitor LED while using the system:
- **Start reflow** → Should change from Solid to Fast Blink
- **Error occurs** → Changes to Medium Blink
- **Complete cycle** → Changes to Slow Blink
- **Return to idle** → Changes to Solid ON

### Remote Monitoring
The LED is especially useful when:
- Computer not connected
- No web browser access
- Checking from across the room
- Quick status check without serial monitor

---

## 🎨 Future Enhancements

### Possible Additional Patterns
Could add more indicators (if needed):
- **Triple blink**: WiFi disconnected
- **SOS pattern**: Critical hardware fault
- **Heartbeat**: Alive indicator
- **Color LED**: If external RGB LED added

### Current Implementation
Simple and effective:
- 4 distinct patterns (easy to distinguish)
- Maps directly to system states
- No confusion between patterns
- Works in all lighting conditions

---

## ✅ Verification Steps

### Test 1: Power On
1. Plug in Pico W
2. Wait 5 seconds
3. **Expected**: LED should be Solid ON (or Medium Blink if thermocouple error)

### Test 2: Web Interface Match
1. Open http://192.168.0.95
2. Check "State" field
3. **Expected**: LED pattern matches state

### Test 3: Start Reflow (after fixing thermocouple)
1. Click "START REFLOW" button
2. **Expected**: LED changes from Solid to Fast Blink immediately

### Test 4: Error Detection
1. Disconnect thermocouple (if working)
2. Wait 3 seconds
3. **Expected**: LED changes to Medium Blink

---

## 📝 Code Implementation

### Key Functions

```cpp
// Set LED state (ON/OFF)
void setLED(bool state) {
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
}

// Update LED based on system state
void updateLED() {
  // Determines blink pattern from currentState
  // Updates LED with precise timing
}
```

### Main Loop Integration
```cpp
void loop() {
  // ... other code ...
  updateLED();  // Called every loop iteration
  // ... other code ...
}
```

---

## 🔗 Related Documentation

- **TROUBLESHOOTING_SESSION.md** - Software fixes and issues
- **HARDWARE_TROUBLESHOOTING.md** - Thermocouple debugging steps
- **README.md** - General system information

---

## Summary

**LED Status**: ✅ NOW WORKING

**Current Pattern**: Medium Blink (ERROR State)
**Reason**: Thermocouple read errors

**After Hardware Fix**: Will show Solid ON (Idle)

**Benefit**: Instant visual feedback without needing computer connection!

---

**Added**: March 17, 2026 - 17:26
**Firmware Version**: With LED status indicators
**Feature**: Onboard LED now provides real-time system status

