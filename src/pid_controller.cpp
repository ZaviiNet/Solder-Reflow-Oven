/*
 * PID Controller Implementation
 */

#include "pid_controller.h"
#include "pins.h"
#include "config.h"
#include "temperature.h"

// Forward declarations
extern ReflowState currentState;
extern void saveConfigToEEPROM();

// PID Variables
static double Setpoint = 0, Input = 0, Output = 0;
static double Kp = 0.05, Ki = 0.001, Kd = 0.75;  // Conservative defaults
static PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// PID Auto-Tuning Variables
static bool pidTuningActive = false;
static int tuningStep = 0;
static double tuningTargetTemp[] = {100.0, 150.0, 200.0};
static int tuningTempIndex = 0;
static double tuningAmplitude = 5.0;
static unsigned long tuningStartTime = 0;
static unsigned long tuningStepStartTime = 0;
static unsigned long oscillationStartTime = 0;
static double lastPeakTemp = 0;
static double lastValleyTemp = 0;
static unsigned long lastPeakTime = 0;
static unsigned long prevPeakTime = 0;
static unsigned long lastValleyTime = 0;
static int oscillationCount = 0;
static double sumOscillationPeriod = 0;
static double sumOscillationAmplitude = 0;
static bool waitingForPeak = true;
static double stepSumPeriod = 0;
static double stepSumAmplitude = 0;
static int stepOscillationCount = 0;
static double tuningKu = 0;
static double tuningPu = 0;
static double calculatedKp = 0;
static double calculatedKi = 0;
static double calculatedKd = 0;

// Console Log
static String consoleLog[MAX_CONSOLE_LINES];
static int consoleLogIndex = 0;

void addConsoleLog(String message) {
  if (consoleLogIndex < MAX_CONSOLE_LINES) {
    consoleLog[consoleLogIndex] = message;
    consoleLogIndex++;
  } else {
    // Shift array
    for (int i = 0; i < MAX_CONSOLE_LINES - 1; i++) {
      consoleLog[i] = consoleLog[i + 1];
    }
    consoleLog[MAX_CONSOLE_LINES - 1] = message;
  }
  Serial.println("[CONSOLE] " + message);
}

void clearConsoleLog() {
  consoleLogIndex = 0;
}

String* getConsoleLog() {
  return consoleLog;
}

int getConsoleLogIndex() {
  return consoleLogIndex;
}

void initPIDController() {
  myPID.SetOutputLimits(0, 1);
  myPID.SetMode(AUTOMATIC);
  Serial.println("PID controller initialized");
}

double getPIDKp() { return Kp; }
double getPIDKi() { return Ki; }
double getPIDKd() { return Kd; }

void setPIDTunings(double kp, double ki, double kd) {
  Kp = kp;
  Ki = ki;
  Kd = kd;
  myPID.SetTunings(Kp, Ki, Kd);
}

double getPIDSetpoint() { return Setpoint; }
void setPIDSetpoint(double setpoint) { Setpoint = setpoint; }

double getPIDOutput() { return Output; }
void setPIDOutput(double output) { Output = output; }

double getPIDInput() { return Input; }
void setPIDInput(double input) { Input = input; }

bool computePID() { return myPID.Compute(); }

void setPIDMode(int mode) { myPID.SetMode(mode); }

PID* getPIDInstance() { return &myPID; }

bool isPIDTuningActive() { return pidTuningActive; }

void startPIDTuning() {
  if (currentState == IDLE || currentState == COMPLETE || currentState == ERROR_STATE) {
    addConsoleLog("=== PID Auto-Tuning Started ===");
    addConsoleLog("This will test at 3 temperatures: 100C, 150C, 200C");
    addConsoleLog("Each test creates controlled oscillations to measure system response");
    addConsoleLog("");

    pidTuningActive = true;
    tuningStep = 0;
    tuningTempIndex = 0;
    tuningStartTime = millis();
    tuningStepStartTime = millis();
    oscillationCount = 0;
    sumOscillationPeriod = 0;
    sumOscillationAmplitude = 0;
    stepSumPeriod = 0;
    stepSumAmplitude = 0;
    stepOscillationCount = 0;
    prevPeakTime = 0;
    consoleLogIndex = 0;

    Output = 0;
    myPID.SetMode(MANUAL);

    currentState = PID_TUNING;
    addConsoleLog("Step 1/6: Heating to " + String(tuningTargetTemp[0]) + "C...");
  }
}

void stopPIDTuning() {
  addConsoleLog("PID Tuning stopped by user");
  pidTuningActive = false;
  digitalWrite(SSR_PIN, LOW);
  Output = 0;
  Setpoint = 0;
  myPID.SetMode(AUTOMATIC);
  currentState = IDLE;
}

static void calculatePIDFromAutoTune() {
  addConsoleLog("");
  addConsoleLog("=== Calculating PID Parameters ===");

  if (oscillationCount < 1) {
    addConsoleLog("ERROR: Not enough oscillation data collected");
    addConsoleLog("Current PID values unchanged");
    addConsoleLog("Old Kp: " + String(Kp, 3) + "  Ki: " + String(Ki, 3) + "  Kd: " + String(Kd, 3));
    return;
  }

  double avgPeriod = sumOscillationPeriod / oscillationCount / 1000.0;
  double avgAmplitude = sumOscillationAmplitude / oscillationCount;

  addConsoleLog("Average oscillation period: " + String(avgPeriod, 2) + " seconds");
  addConsoleLog("Average amplitude: " + String(avgAmplitude, 2) + " C");

  double relayAmplitude = 1.0;
  tuningKu = (4.0 * relayAmplitude) / (PI * avgAmplitude);
  tuningPu = avgPeriod;

  addConsoleLog("Ultimate gain (Ku): " + String(tuningKu, 3));
  addConsoleLog("Ultimate period (Pu): " + String(tuningPu, 2) + "s");
  addConsoleLog("");

  // Ziegler-Nichols tuning rules
  calculatedKp = 0.6 * tuningKu;
  calculatedKi = 1.2 * tuningKu / tuningPu;
  calculatedKd = 0.075 * tuningKu * tuningPu;

  addConsoleLog("=== PID Values: Old vs New (Ziegler-Nichols) ===");
  addConsoleLog("         Old      New");
  addConsoleLog("  Kp:  " + String(Kp, 3) + "  ->  " + String(calculatedKp, 3));
  addConsoleLog("  Ki:  " + String(Ki, 3) + "  ->  " + String(calculatedKi, 3));
  addConsoleLog("  Kd:  " + String(Kd, 3) + "  ->  " + String(calculatedKd, 3));
  addConsoleLog("");
  addConsoleLog("Applying new PID values...");

  Kp = calculatedKp;
  Ki = calculatedKi;
  Kd = calculatedKd;
  myPID.SetTunings(Kp, Ki, Kd);
  saveConfigToEEPROM();

  addConsoleLog("PID values saved to flash.");
  addConsoleLog("");
  addConsoleLog("=== Auto-Tuning Complete ===");
}

void updatePIDTuningStateMachine() {
  if (!pidTuningActive) return;

  Input = getTemperatureInput();
  unsigned long elapsed = millis() - tuningStepStartTime;
  double targetTemp = tuningTargetTemp[tuningTempIndex];

  // Step 0, 2, 4: Heat to target temperature
  if (tuningStep % 2 == 0) {
    Setpoint = targetTemp;

    if (Input < targetTemp - 2.0) {
      digitalWrite(SSR_PIN, HIGH);
      Output = 1;
    } else if (Input >= targetTemp - 2.0) {
      addConsoleLog("Old PID values -> Kp: " + String(Kp, 3) + ", Ki: " + String(Ki, 3) + ", Kd: " + String(Kd, 3));
      tuningStep++;
      tuningStepStartTime = millis();
      oscillationStartTime = millis();
      stepSumPeriod = 0;
      stepSumAmplitude = 0;
      stepOscillationCount = 0;
      prevPeakTime = 0;
      waitingForPeak = true;
      lastPeakTemp = Input;
      lastValleyTemp = Input;

      addConsoleLog("Target " + String(targetTemp) + "C reached. Starting oscillation test...");
      addConsoleLog("Step " + String(tuningStep/2 + 1) + "/6: Testing at " + String(targetTemp) + "C");
    }
  }
  // Step 1, 3, 5: Oscillation test
  else {
    if (Input < targetTemp - tuningAmplitude) {
      digitalWrite(SSR_PIN, HIGH);
      Output = 1;

      // Detect valley
      if (!waitingForPeak && Input < lastValleyTemp) {
        lastValleyTemp = Input;
        lastValleyTime = millis();
      } else if (!waitingForPeak && Input > lastValleyTemp + 1.0) {
        waitingForPeak = true;
        lastPeakTemp = Input;
      }
    } else if (Input > targetTemp + tuningAmplitude) {
      digitalWrite(SSR_PIN, LOW);
      Output = 0;

      // Detect peak
      if (waitingForPeak && Input > lastPeakTemp) {
        lastPeakTemp = Input;
        lastPeakTime = millis();
      } else if (waitingForPeak && Input < lastPeakTemp - 1.0) {
        // Peak confirmed
        if (prevPeakTime > 0) {
          unsigned long period = lastPeakTime - prevPeakTime;
          double amplitude = lastPeakTemp - lastValleyTemp;

          if (period > 1000 && period < 300000 && amplitude > 1.0) {
            stepSumPeriod += period;
            stepSumAmplitude += amplitude;
            stepOscillationCount++;
            sumOscillationPeriod += period;
            sumOscillationAmplitude += amplitude;
            oscillationCount++;

            addConsoleLog("  Oscillation #" + String(oscillationCount) +
                         ": Period=" + String(period/1000.0, 1) + "s, Amplitude=" +
                         String(amplitude, 1) + "C");
          }
        }

        prevPeakTime = lastPeakTime;
        waitingForPeak = false;
        lastValleyTemp = Input;
      }
    }

    // Need at least 3 oscillations per step, or timeout after 10 minutes
    if (stepOscillationCount >= 3 || elapsed > 600000) {
      if (stepOscillationCount >= 3) {
        addConsoleLog("Oscillation test complete at " + String(targetTemp) + "C");
      } else {
        addConsoleLog("Timeout - moving to next temperature");
      }

      // Log per-step PID estimate
      if (stepOscillationCount >= 1) {
        double stepAvgPeriod = stepSumPeriod / stepOscillationCount / 1000.0;
        double stepAvgAmplitude = stepSumAmplitude / stepOscillationCount;
        double stepKu = (4.0 * 1.0) / (PI * stepAvgAmplitude);
        double stepKp = 0.6 * stepKu;
        double stepKi = 1.2 * stepKu / stepAvgPeriod;
        double stepKd = 0.075 * stepKu * stepAvgPeriod;
        addConsoleLog("  Step PID estimate @ " + String(targetTemp, 0) + "C:");
        addConsoleLog("    New Kp: " + String(stepKp, 3) +
                      "  Ki: " + String(stepKi, 3) +
                      "  Kd: " + String(stepKd, 3));
      }
      addConsoleLog("");

      // Move to next temperature or complete
      tuningTempIndex++;
      if (tuningTempIndex < 3) {
        tuningStep++;
        tuningStepStartTime = millis();
        addConsoleLog("Step " + String(tuningStep/2 + 2) + "/6: Heating to " +
                     String(tuningTargetTemp[tuningTempIndex]) + "C...");
      } else {
        // All tests complete
        calculatePIDFromAutoTune();
        pidTuningActive = false;
        digitalWrite(SSR_PIN, LOW);
        Output = 0;
        Setpoint = 0;
        myPID.SetMode(AUTOMATIC);
        currentState = IDLE;
      }
    }
  }
}

