# PID Auto-Tuning Feature

## Quick Start Guide

### Step 1: Prepare
- ✅ Empty the oven (no PCBs inside)
- ✅ Ensure 15-30 minutes available
- ✅ Open web interface

### Step 2: Start Auto-Tuning
1. Navigate to **⚙️ PID Tuning** section
2. Click **"🔧 Auto-Tune PID"** button
3. Confirm the dialog
4. Console log will show automatically

### Step 3: Monitor Progress
Watch the console log for real-time updates:
```
=== PID Auto-Tuning Started ===
Step 1/6: Heating to 100C...
Target 100C reached. Starting oscillation test...
  Oscillation #1: Period=45.2s, Amplitude=8.3C
  Oscillation #2: Period=46.1s, Amplitude=7.9C
  Oscillation #3: Period=45.8s, Amplitude=8.1C
...
=== Recommended PID Values ===
Kp = 0.094
Ki = 0.002
Kd = 0.539
```

### Step 4: Apply Results
1. **Copy** the recommended Kp, Ki, Kd values from console
2. **Paste** into the PID input fields above
3. Click **"Save PID Settings"**
4. Values are now saved to EEPROM!

## Feature Overview

### What It Does
Automatically determines optimal PID parameters for your specific oven by:
1. Testing at multiple temperatures
2. Measuring system response characteristics
3. Calculating optimal control parameters
4. Displaying results in an easy-to-use console

### Process Flow
```
START → Heat to 100°C → Oscillation Test → Heat to 150°C → Oscillation Test
                                                                    ↓
COMPLETE ← Display Results ← Heat to 200°C ← Oscillation Test ← ─┘
```

### User Interface

**Before Auto-Tuning:**
```
┌─────────────────────────────────────────┐
│ ⚙️ PID Tuning                           │
├─────────────────────────────────────────┤
│ Kp: [2.0]  Ki: [5.0]  Kd: [1.0]        │
│                                         │
│ [Save PID Settings]  [🔧 Auto-Tune PID]│
│                                         │
│ Console Log        [Show/Hide Console] │
│ ┌─────────────────────────────────────┐ │
│ │ (hidden)                            │ │
│ └─────────────────────────────────────┘ │
└─────────────────────────────────────────┘
```

**During Auto-Tuning:**
```
┌─────────────────────────────────────────┐
│ ⚙️ PID Tuning                           │
├─────────────────────────────────────────┤
│ Kp: [2.0]  Ki: [5.0]  Kd: [1.0]        │
│                                         │
│ [Save PID Settings]  [⏹ Stop Auto-Tune]│
│                                         │
│ Console Log        [Show/Hide Console] │
│ ┌─────────────────────────────────────┐ │
│ │ === PID Auto-Tuning Started ===    │ │
│ │ Step 2/6: Testing at 100C          │ │
│ │   Oscillation #2: Period=45s       │ │
│ │ ...                                 │ │
│ └─────────────────────────────────────┘ │
└─────────────────────────────────────────┘
```

## Technical Details

### Algorithm: Relay-Based Method
- **Method**: Åström-Hägglund relay feedback
- **Tuning Rules**: Ziegler-Nichols
- **Test Points**: 100°C, 150°C, 200°C
- **Oscillations**: 3+ per temperature
- **Duration**: 5-10 minutes per temperature

### Calculations
```
Ultimate Gain:    Ku = (4 × relay_amp) / (π × osc_amp)
Ultimate Period:  Pu = average oscillation period

PID Parameters:
  Kp = 0.6 × Ku          (proportional)
  Ki = 1.2 × Ku / Pu     (integral)
  Kd = 0.075 × Ku × Pu   (derivative)
```

### Safety Features
- ✅ Can be stopped at any time
- ✅ 10-minute timeout per step
- ✅ Thermocouple error detection
- ✅ Emergency stop button remains active
- ✅ SSR forced off on errors

## API Endpoints

### Start/Stop Tuning
```http
POST /api/tune-pid
Response: {"status": "tuning_started"} or {"status": "tuning_stopped"}
```

### Get Console Log
```http
GET /api/console
Response: ["line 1", "line 2", "line 3", ...]
```

### Get Status (includes tuning state)
```http
GET /api/status
Response: {
  "temp": 25.3,
  "setpoint": 100.0,
  "state": "PID_TUNING",
  "tuning": true,
  ...
}
```

## Code Structure

### New State
```cpp
enum ReflowState {
  IDLE, PREHEAT, SOAK, REFLOW, COOLDOWN, COMPLETE, ERROR_STATE,
  PID_TUNING  // NEW!
};
```

### Key Functions
```cpp
void startPIDTuning()              // Initiates auto-tuning
void stopPIDTuning()               // Stops tuning process
void updatePIDTuningStateMachine() // Runs tuning algorithm
void calculatePIDFromAutoTune()    // Computes PID values
void addConsoleLog(String msg)     // Adds message to console
```

### Data Structures
```cpp
// Tuning variables
double tuningTargetTemp[] = {100.0, 150.0, 200.0};
int tuningStep;           // Current step in process
int oscillationCount;     // Number of oscillations recorded
double sumOscillationPeriod;     // Accumulated period data
double sumOscillationAmplitude;  // Accumulated amplitude data
double tuningKu, tuningPu;       // Ultimate gain and period

// Console log
String consoleLog[50];    // Circular buffer for log lines
int consoleLogIndex;      // Current position in buffer
```

## Advantages

✨ **Automatic** - No manual trial-and-error tuning required
✨ **Optimized** - Values tailored to your specific oven
✨ **Easy** - One-click operation with clear results
✨ **Safe** - Multiple safety features and timeouts
✨ **Educational** - Console shows exactly what's happening
✨ **Documented** - Clear explanation of the algorithm

## Limitations

⚠️ **Time Required** - Takes 15-30 minutes to complete
⚠️ **Hardware Specific** - Results depend on oven being empty
⚠️ **Conservative** - Values prioritize stability over speed
⚠️ **May Need Refinement** - Manual fine-tuning may still be beneficial
⚠️ **Thermal Mass** - Results differ with vs. without PCBs in oven

## Future Enhancements

Potential improvements for future versions:
- [ ] Adaptive tuning with PCB thermal mass
- [ ] Multiple tuning profiles (aggressive/balanced/conservative)
- [ ] Historical tuning results storage
- [ ] Graphical display of oscillation data
- [ ] Export tuning data for analysis
- [ ] Email/notification when tuning completes

## References

- Åström, K. J., & Hägglund, T. (1984). "Automatic tuning of simple regulators"
- Ziegler, J. G., & Nichols, N. B. (1942). "Optimum settings for automatic controllers"
- Arduino PID Library: https://github.com/br3ttb/Arduino-PID-Library

---

**For detailed usage instructions, see [PID_TUNING_GUIDE.md](PID_TUNING_GUIDE.md)**
