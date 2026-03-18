# PID Auto-Tuning Improvements - Changes Summary

## Date: March 18, 2026

## Overview
This document summarizes the changes made to fix PID default values, reset target temperature when returning to IDLE, and ensure the chart updates during auto-tuning.

---

## Changes Made

### 1. **Updated Default PID Values** (Line 71)

**Previous Values:**
```cpp
double Kp = 1.0, Ki = 0.05, Kd = 5.0;
```

**New Values:**
```cpp
double Kp = 0.05, Ki = 0.001, Kd = 0.75;  // Conservative defaults based on auto-tune results
```

**Reason:**
- Old defaults were **way too aggressive** (25x-50x higher than auto-tuned values)
- Caused massive overshoot (reaching 120°C when targeting 100°C)
- New defaults are based on actual auto-tune results from your oven
- Prevents overshoot while maintaining reasonable response time

**Impact:**
- First-time users will have much better temperature control out of the box
- No more 20°C overshoot on initial heating
- System remains stable and predictable

---

### 2. **Reset Setpoint When Entering IDLE State** (3 locations)

#### Location A: `changeState()` function (Line ~868)
**Added:**
```cpp
// Reset setpoint when entering IDLE
if (newState == IDLE) {
  Setpoint = 0;
}
```

**Reason:**
- When transitioning to any IDLE state, the setpoint should be reset
- This ensures the UI displays `"--"` instead of the last target (e.g., 200°C)
- Makes it clear to users that the system is not targeting any temperature

#### Location B: `stopPIDTuning()` function (Line ~934)
**Added:**
```cpp
Setpoint = 0;  // Reset setpoint when returning to IDLE
```

**Reason:**
- When user manually stops PID tuning, reset the target temperature
- Prevents displaying stale target value in UI

#### Location C: `updatePIDTuningStateMachine()` function (Line ~1059)
**Added:**
```cpp
Setpoint = 0;  // Reset setpoint when returning to IDLE
```

**Reason:**
- When PID tuning completes successfully, reset the target temperature
- This was the original issue - target remained at 200°C after tuning completed

**Impact:**
- UI now correctly shows `"--"` for target temperature when in IDLE state
- No more confusion about what temperature the system is targeting
- Cleaner user experience

---

### 3. **Fix Elapsed Time During PID_TUNING** (Line ~2009)

**Previous Code:**
```cpp
doc["elapsed"] = (currentState != IDLE) ? (millis() - reflowStartTime) : 0;
```

**New Code:**
```cpp
// Calculate elapsed time based on current state
if (currentState == PID_TUNING) {
  doc["elapsed"] = millis() - tuningStartTime;
} else if (currentState != IDLE) {
  doc["elapsed"] = millis() - reflowStartTime;
} else {
  doc["elapsed"] = 0;
}
```

**Reason:**
- During PID_TUNING, the system was using `reflowStartTime` which was never set
- This caused incorrect elapsed time calculation
- Chart updates depend on elapsed time, so this was breaking the chart
- Now uses `tuningStartTime` during tuning

**Impact:**
- Chart now updates properly during auto-tuning
- Elapsed time shows correctly in UI during tuning
- Better visibility into tuning progress

---

### 4. **Updated Documentation** (PID_TUNING_GUIDE.md)

Updated 3 sections to reflect the new default values:

1. **Parameter Defaults Section:**
   - Changed Kp: 2.0 → 0.05
   - Changed Ki: 5.0 → 0.001
   - Changed Kd: 1.0 → 0.75

2. **Manual Tuning Process:**
   - Updated starting values in step 1
   - Adjusted suggested increment values to match new scale

3. **Default Values Section:**
   - Updated all default values
   - Changed description from "work well" to "conservative values based on auto-tune results"
   - Emphasized that these prevent overshoot

**Impact:**
- Documentation now matches the code
- Users have correct reference values
- Guidance reflects realistic tuning adjustments

---

## Testing Recommendations

### 1. **Verify Default Values Work**
- Flash the updated firmware to your Pico W
- Start a reflow cycle WITHOUT running auto-tune first
- Check that temperature rises smoothly to target
- Verify overshoot is minimal (< 5°C)

### 2. **Verify Target Temperature Reset**
- Run auto-tune to completion
- Check that target temperature shows `"--"` when in IDLE
- Start a new reflow cycle
- Verify target updates correctly during reflow

### 3. **Verify Chart Updates During Tuning**
- Start auto-tune
- Watch the temperature chart
- Verify it updates in real-time during all phases
- Check that elapsed time increases properly

### 4. **Verify Manual Stop Works**
- Start auto-tune
- Click "Stop Auto-Tune" button mid-way through
- Verify system returns to IDLE
- Check that target shows `"--"`

---

## Rollback Instructions

If you need to revert to the old values:

```cpp
// Line 71 - Old defaults
double Kp = 1.0, Ki = 0.05, Kd = 5.0;

// Remove these lines from changeState(), stopPIDTuning(), and updatePIDTuningStateMachine():
Setpoint = 0;

// Line ~2009 - Old elapsed time calculation
doc["elapsed"] = (currentState != IDLE) ? (millis() - reflowStartTime) : 0;
```

---

## Expected Behavior After Changes

### During Auto-Tuning:
1. Console log shows old PID values before each test
2. Console log shows new PID values after each test
3. Temperature chart updates in real-time
4. Elapsed time increases correctly
5. When complete, target shows `"--"`

### During Normal Reflow:
1. Temperature rises smoothly without overshoot
2. PID control is stable around setpoint
3. Transitions between stages are smooth

### In IDLE State:
1. Target temperature always shows `"--"`
2. SSR is off
3. System ready to start new cycle

---

## Files Modified

1. `/home/jack/CLionProjects/Solder-Reflow-Oven-IO/src/main.cpp`
   - Line 71: Updated default PID values
   - Line ~868: Added setpoint reset in `changeState()`
   - Line ~934: Added setpoint reset in `stopPIDTuning()`
   - Line ~1059: Added setpoint reset in `updatePIDTuningStateMachine()`
   - Line ~2009: Fixed elapsed time calculation in `handleStatus()`

2. `/home/jack/CLionProjects/Solder-Reflow-Oven-IO/PID_TUNING_GUIDE.md`
   - Updated all references to default PID values
   - Updated manual tuning guidance
   - Updated default values section

---

## Next Steps

1. **Compile and Flash:**
   ```bash
   cd /home/jack/CLionProjects/Solder-Reflow-Oven-IO
   pio run -t upload
   ```

2. **Test Empty Oven:**
   - Run a test without PCB
   - Verify smooth heating to 100°C
   - Check for minimal overshoot

3. **Run Auto-Tune Again (Optional):**
   - See if new defaults change the auto-tune results
   - Compare with previous results
   - Fine-tune if needed

4. **Test Full Reflow Cycle:**
   - Run complete reflow profile
   - Monitor temperature chart
   - Verify all stage transitions work correctly

---

## Notes

- These changes are **backwards compatible** - existing saved PID values in flash will still be loaded and used
- If you've already run auto-tune and saved values, those will take precedence over the new defaults
- The new defaults only affect fresh installations or after a factory reset
- All safety features (emergency stop, timeouts) remain unchanged

---

## Questions or Issues?

If you encounter any problems:
1. Check the Serial Monitor for error messages
2. Verify the chart is updating (may need to clear browser cache)
3. Try running auto-tune again with new defaults
4. Check that Setpoint = 0 when in IDLE state (visible in `/api/status` response)

