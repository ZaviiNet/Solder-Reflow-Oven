# PID Auto-Tune Library Integration Guide

## Overview

The project includes the `PID-AutoTune` library (by Brett Beauregard) which provides a more robust and tested auto-tuning implementation compared to a custom solution.

## Current Status

**Current Implementation**: Custom relay-based auto-tune (manual implementation)
**Available Library**: `PID_AutoTune_v0` (br3ttb/PID-AutoTune @ ^1.0.0)

## Why Use the Library?

### Advantages of PID_AutoTune Library

1. **Proven Algorithm**: Well-tested implementation of Åström-Hägglund relay feedback method
2. **Simpler Code**: Less custom code to maintain
3. **Configurable**: Easy to adjust parameters (noise band, lookback time, output step)
4. **Automatic Detection**: Automatically detects oscillation completion
5. **Multiple Control Types**: Supports both PI and PID tuning

### Comparison

| Feature | Custom Implementation | PID_AutoTune Library |
|---------|----------------------|----------------------|
| **Lines of Code** | ~300 lines | ~50 lines (using library) |
| **Maintenance** | Manual | Library maintainer |
| **Oscillation Detection** | Manual peak/valley tracking | Automatic |
| **Temperature Points** | Fixed 3 points (100, 150, 200°C) | Configurable |
| **Noise Handling** | Basic threshold | Configurable noise band |
| **Tuning Methods** | Ziegler-Nichols only | Ziegler-Nichols + custom |

## Library API Reference

### Constructor
```cpp
PID_ATune(double* Input, double* Output)
```

### Main Functions
```cpp
int Runtime()              // Call repeatedly; returns 1 when done
void Cancel()              // Stop auto-tuning
```

### Configuration
```cpp
void SetOutputStep(double)     // Output step size (default: 30)
void SetControlType(int)       // 0=PI, 1=PID (default: 0)
void SetLookbackSec(int)       // Lookback window in seconds (default: 10)
void SetNoiseBand(double)      // Noise threshold (default: 0.5)
```

### Get Results
```cpp
double GetKp()    // Get calculated Kp
double GetKi()    // Get calculated Ki
double GetKd()    // Get calculated Kd (0 for PI mode)
```

## Integration Example

### Basic Usage Pattern

```cpp
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>

// PID variables
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Auto-tune object
PID_ATune autoTuner(&Input, &Output);

// Auto-tune state
bool tuningActive = false;
unsigned long tuneStartTime = 0;

void setup() {
  myPID.SetMode(AUTOMATIC);
  myPID.SetSampleTime(1000);

  // Configure auto-tuner
  autoTuner.SetOutputStep(50);        // 50% output step
  autoTuner.SetControlType(1);        // 1 = PID mode
  autoTuner.SetLookbackSec(15);       // 15 second lookback
  autoTuner.SetNoiseBand(1.0);        // 1°C noise band
}

void loop() {
  // Read temperature
  Input = readTemperature();

  if (tuningActive) {
    // Run auto-tuner
    int result = autoTuner.Runtime();

    if (result != 0) {
      // Tuning complete!
      tuningActive = false;

      // Get results
      Kp = autoTuner.GetKp();
      Ki = autoTuner.GetKi();
      Kd = autoTuner.GetKd();

      // Apply to PID
      myPID.SetTunings(Kp, Ki, Kd);
      myPID.SetMode(AUTOMATIC);

      Serial.println("Tuning complete!");
      Serial.print("Kp: "); Serial.println(Kp);
      Serial.print("Ki: "); Serial.println(Ki);
      Serial.print("Kd: "); Serial.println(Kd);
    }

    // Output is controlled by auto-tuner
    digitalWrite(SSR_PIN, Output > 0 ? HIGH : LOW);

  } else {
    // Normal PID operation
    myPID.Compute();
    digitalWrite(SSR_PIN, Output > 0 ? HIGH : LOW);
  }
}

void startAutoTune() {
  tuningActive = true;
  tuneStartTime = millis();

  // Set target temperature for tuning
  Input = readTemperature();
  Setpoint = Input + 50;  // Tune 50°C above current temp

  // Stop normal PID
  myPID.SetMode(MANUAL);
  Output = 0;

  Serial.println("Auto-tune started");
}

void stopAutoTune() {
  autoTuner.Cancel();
  tuningActive = false;
  myPID.SetMode(AUTOMATIC);
  Serial.println("Auto-tune cancelled");
}
```

## Recommended Configuration for Reflow Oven

### For ESP8266 (1 Hz PID loop)
```cpp
autoTuner.SetOutputStep(100);      // Full on/off for SSR
autoTuner.SetControlType(1);       // PID mode
autoTuner.SetLookbackSec(20);      // 20 second lookback
autoTuner.SetNoiseBand(2.0);       // 2°C noise tolerance
```

### For Pico W (10 Hz PID loop)
```cpp
autoTuner.SetOutputStep(100);      // Full on/off for SSR
autoTuner.SetControlType(1);       // PID mode
autoTuner.SetLookbackSec(15);      // 15 second lookback
autoTuner.SetNoiseBand(1.0);       // 1°C noise tolerance (tighter)
```

## Multi-Temperature Tuning

For better results across the reflow temperature range, you can run auto-tune at multiple setpoints:

```cpp
enum TuneState {
  TUNE_IDLE,
  TUNE_HEAT,
  TUNE_RUN,
  TUNE_NEXT,
  TUNE_COMPLETE
};

TuneState tuneState = TUNE_IDLE;
int tuneStepIndex = 0;
double tuneTemps[] = {100, 150, 200};  // Test temperatures
int numTuneSteps = 3;

double sumKp = 0, sumKi = 0, sumKd = 0;
int validResults = 0;

void multiTempTuneLoop() {
  switch (tuneState) {
    case TUNE_HEAT:
      // Heat to target temperature
      Setpoint = tuneTemps[tuneStepIndex];
      myPID.Compute();
      digitalWrite(SSR_PIN, Output > 0 ? HIGH : LOW);

      if (abs(Input - Setpoint) < 5.0) {
        // Temperature reached, start tuning
        tuneState = TUNE_RUN;
        autoTuner.SetOutputStep(100);
        Serial.print("Starting tune at ");
        Serial.print(Setpoint);
        Serial.println("°C");
      }
      break;

    case TUNE_RUN:
      int result = autoTuner.Runtime();
      digitalWrite(SSR_PIN, Output > 0 ? HIGH : LOW);

      if (result != 0) {
        // Tuning complete for this temperature
        sumKp += autoTuner.GetKp();
        sumKi += autoTuner.GetKi();
        sumKd += autoTuner.GetKd();
        validResults++;

        Serial.print("Completed: Kp=");
        Serial.print(autoTuner.GetKp());
        Serial.print(" Ki=");
        Serial.print(autoTuner.GetKi());
        Serial.print(" Kd=");
        Serial.println(autoTuner.GetKd());

        tuneState = TUNE_NEXT;
      }
      break;

    case TUNE_NEXT:
      tuneStepIndex++;
      if (tuneStepIndex < numTuneSteps) {
        tuneState = TUNE_HEAT;
      } else {
        tuneState = TUNE_COMPLETE;

        // Calculate average values
        if (validResults > 0) {
          Kp = sumKp / validResults;
          Ki = sumKi / validResults;
          Kd = sumKd / validResults;

          myPID.SetTunings(Kp, Ki, Kd);

          Serial.println("=== Multi-Temperature Tuning Complete ===");
          Serial.print("Average Kp: "); Serial.println(Kp);
          Serial.print("Average Ki: "); Serial.println(Ki);
          Serial.print("Average Kd: "); Serial.println(Kd);
        }

        myPID.SetMode(AUTOMATIC);
      }
      break;
  }
}
```

## Advantages Over Custom Implementation

### 1. Automatic Oscillation Detection
The library automatically detects when enough oscillations have occurred, no need for manual peak/valley counting.

### 2. Noise Filtering
The configurable noise band prevents false peak detection from sensor noise.

### 3. Adaptive Lookback
The lookback window automatically adjusts based on the sample time.

### 4. Proven Ziegler-Nichols Rules
Uses standard ZN rules with proper calculations:
- **PI Mode**: Kp = 0.4*Ku, Ki = 0.48*Ku/Pu
- **PID Mode**: Kp = 0.6*Ku, Ki = 1.2*Ku/Pu, Kd = 0.075*Ku*Pu

### 5. Less Code to Maintain
The library handles all the complex state management and calculation logic.

## Migration Path

### Phase 1: Test Library (Side-by-side)
Add library-based tuning as a new option while keeping existing implementation.

### Phase 2: Compare Results
Run both methods and compare PID values and performance.

### Phase 3: Replace
Once validated, replace custom implementation with library.

### Phase 4: Optimize
Fine-tune library parameters for specific oven characteristics.

## Sample Integration for Reflow Oven

See the refactored implementation in:
- `/src/main_with_autotune_lib.cpp` (example file)
- Or modify existing `/src/main.cpp` to use the library

## Tips and Best Practices

### 1. Choose Appropriate Test Temperature
- Too low: Slow response, long tuning time
- Too high: Risk of overshoot, safety concern
- **Recommended**: Middle of operating range (150-180°C for reflow)

### 2. Output Step Size
- For SSR control: Use 100 (full on/off)
- For PWM control: Use 50-80 (percentage)

### 3. Lookback Time
- Faster systems: 10-15 seconds
- Slower systems: 20-30 seconds
- Rule of thumb: ~2-3x expected oscillation period

### 4. Noise Band
- Good sensor: 0.5-1.0°C
- Noisy sensor: 2.0-3.0°C
- MAX31855: 0.5-1.0°C (very clean)

### 5. Timeout
Add a timeout to prevent infinite tuning:
```cpp
unsigned long tuneStartTime = millis();
unsigned long TUNE_TIMEOUT = 600000;  // 10 minutes

if (tuningActive && (millis() - tuneStartTime > TUNE_TIMEOUT)) {
  autoTuner.Cancel();
  tuningActive = false;
  Serial.println("Tuning timeout!");
}
```

## Web Interface Integration

### JavaScript Button Handler
```javascript
function tunePID() {
  if (confirm('Start auto-tuning? This will take 5-10 minutes.')) {
    fetch('/api/tune-start', { method: 'POST' })
      .then(r => r.json())
      .then(data => {
        console.log('Tuning started');
        startTuneMonitoring();
      });
  }
}
```

### API Endpoint
```cpp
void handleTuneStart() {
  if (currentState != IDLE) {
    server.send(400, "application/json",
                "{\"error\":\"Must be IDLE to start tuning\"}");
    return;
  }

  startAutoTune();
  server.send(200, "application/json",
              "{\"status\":\"tuning_started\"}");
}
```

## Conclusion

The PID_AutoTune library provides a robust, tested, and maintainable solution for automatic PID tuning. While the custom implementation works, using the library offers:

- ✅ **Less code complexity**
- ✅ **Better noise handling**
- ✅ **Proven algorithm**
- ✅ **Easier configuration**
- ✅ **Active maintenance**

**Recommendation**: Migrate to the library-based implementation for improved reliability and maintainability.

## References

- [PID Library Documentation](https://github.com/br3ttb/Arduino-PID-Library)
- [PID AutoTune Library](https://github.com/br3ttb/Arduino-PID-AutoTune-Library)
- [Åström-Hägglund Paper](https://www.sciencedirect.com/science/article/pii/0005109884900204)
- Ziegler-Nichols Tuning Rules

