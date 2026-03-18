/*
 * State Machine Implementation
 */

#include "state_machine.h"
#include "pins.h"
#include "temperature.h"
#include "pid_controller.h"

// Reflow profile parameters
double preheatTemp = 150;
double soakTemp = 180;
double reflowTemp = 230;
double cooldownTemp = 25;
unsigned long preheatTime = 90000;
unsigned long soakTime = 90000;
unsigned long reflowTime = 40000;
unsigned long cooldownTime = 60000;

// Temperature-based transitions
bool useTemperatureBasedTransitions = true;

// State variables
ReflowState currentState = IDLE;
String stateNames[] = {"IDLE", "PREHEAT", "SOAK", "REFLOW", "COOLDOWN", "COMPLETE", "ERROR", "PID_TUNING"};

// Timing
static unsigned long reflowStartTime = 0;
static unsigned long stateStartTime = 0;
static unsigned long tempReachedTime = 0;

// Safety
static bool emergencyStop = false;

// Data logging
static DataPoint dataLog[MAX_DATA_POINTS];
static int dataLogIndex = 0;

void initStateMachine() {
  currentState = IDLE;
  emergencyStop = false;
  Serial.println("State machine initialized");
}

ReflowState getCurrentState() {
  return currentState;
}

void changeState(ReflowState newState) {
  Serial.print("State change: ");
  Serial.print(stateNames[currentState]);
  Serial.print(" -> ");
  Serial.println(stateNames[newState]);

  currentState = newState;
  stateStartTime = millis();
  tempReachedTime = 0;

  if (newState == IDLE) {
    setPIDSetpoint(0);
  }
}

String getStateName(ReflowState state) {
  return stateNames[state];
}

bool isEmergencyStop() {
  return emergencyStop;
}

void emergencyStopReflow() {
  digitalWrite(SSR_PIN, LOW);
  emergencyStop = true;
  setPIDOutput(0);
  setPIDSetpoint(cooldownTemp);
  Serial.println("EMERGENCY STOP - SSR turned OFF");
}

void startReflow() {
  if (currentState == IDLE || currentState == COMPLETE || currentState == ERROR_STATE) {
    Serial.println("Starting reflow process...");
    emergencyStop = false;
    resetThermocoupleErrorCount();
    reflowStartTime = millis();
    dataLogIndex = 0;

    // Reset PID controller
    setPIDOutput(0);
    setPIDMode(MANUAL);
    setPIDMode(AUTOMATIC);
    Serial.println("PID controller reset for new reflow cycle");

    changeState(PREHEAT);
  }
}

void stopReflow() {
  Serial.println("Stopping reflow process...");
  emergencyStopReflow();
  currentState = IDLE;
}

void updateReflowStateMachine() {
  if (emergencyStop) {
    return;
  }

  double Input = getTemperatureInput();
  unsigned long stateElapsed = millis() - stateStartTime;

  switch (currentState) {
    case IDLE:
      break;

    case PREHEAT:
      setPIDSetpoint(preheatTemp);
      if (useTemperatureBasedTransitions) {
        if (Input >= preheatTemp - TEMP_REACH_THRESHOLD) {
          if (tempReachedTime == 0) {
            tempReachedTime = millis();
            Serial.print("PREHEAT temperature reached at ");
            Serial.print(Input);
            Serial.println("°C - starting preheat timer");
          }
          if (millis() - tempReachedTime >= preheatTime) {
            changeState(SOAK);
          }
        } else if (stateElapsed >= MAX_STATE_TIMEOUT) {
          Serial.println("WARNING: PREHEAT timeout - temperature not reached!");
          changeState(SOAK);
        }
      } else {
        if (stateElapsed >= preheatTime) {
          changeState(SOAK);
        }
      }
      break;

    case SOAK:
      setPIDSetpoint(soakTemp);
      if (useTemperatureBasedTransitions) {
        if (Input >= soakTemp - TEMP_REACH_THRESHOLD) {
          if (tempReachedTime == 0) {
            tempReachedTime = millis();
            Serial.print("SOAK temperature reached at ");
            Serial.print(Input);
            Serial.println("°C - starting soak timer");
          }
          if (millis() - tempReachedTime >= soakTime) {
            changeState(REFLOW);
          }
        } else if (stateElapsed >= MAX_STATE_TIMEOUT) {
          Serial.println("WARNING: SOAK timeout - temperature not reached!");
          changeState(REFLOW);
        }
      } else {
        if (stateElapsed >= soakTime) {
          changeState(REFLOW);
        }
      }
      break;

    case REFLOW:
      setPIDSetpoint(reflowTemp);
      if (useTemperatureBasedTransitions) {
        if (Input >= reflowTemp - TEMP_REACH_THRESHOLD) {
          if (tempReachedTime == 0) {
            tempReachedTime = millis();
            Serial.print("REFLOW temperature reached at ");
            Serial.print(Input);
            Serial.println("°C - starting reflow timer");
          }
          if (millis() - tempReachedTime >= reflowTime) {
            changeState(COOLDOWN);
          }
        } else if (stateElapsed >= MAX_STATE_TIMEOUT) {
          Serial.println("WARNING: REFLOW timeout - temperature not reached!");
          changeState(COOLDOWN);
        }
      } else {
        if (stateElapsed >= reflowTime) {
          changeState(COOLDOWN);
        }
      }
      break;

    case COOLDOWN:
      setPIDSetpoint(cooldownTemp);
      digitalWrite(SSR_PIN, LOW);
      if (Input <= cooldownTemp + 10 || stateElapsed >= cooldownTime) {
        changeState(COMPLETE);
      }
      break;

    case COMPLETE:
      setPIDSetpoint(cooldownTemp);
      digitalWrite(SSR_PIN, LOW);
      break;

    case ERROR_STATE:
      setPIDSetpoint(0);
      digitalWrite(SSR_PIN, LOW);
      break;

    case PID_TUNING:
      digitalWrite(SSR_PIN, isPIDTuningActive() ? HIGH : LOW);
      break;
  }

  // Run PID
  if (currentState != COOLDOWN && currentState != COMPLETE && currentState != ERROR_STATE) {
    setPIDInput(Input);
    computePID();

    if (getPIDOutput() > 0.5) {
      digitalWrite(SSR_PIN, HIGH);
    } else {
      digitalWrite(SSR_PIN, LOW);
    }
  } else {
    digitalWrite(SSR_PIN, LOW);
  }

  // Print status
  Serial.print(stateNames[currentState]);
  Serial.print(" | Time: ");
  Serial.print(stateElapsed / 1000);
  Serial.print("s | Temp: ");
  Serial.print(Input);
  Serial.print("°C | Setpoint: ");
  Serial.print(getPIDSetpoint());
  Serial.print("°C | Output: ");
  Serial.print(getPIDOutput());
  Serial.print(" | SSR: ");
  Serial.println(digitalRead(SSR_PIN) ? "ON" : "OFF");
}

void logDataPoint() {
  if (dataLogIndex < MAX_DATA_POINTS) {
    dataLog[dataLogIndex].time = millis() - reflowStartTime;
    dataLog[dataLogIndex].temperature = getTemperatureInput();
    dataLog[dataLogIndex].setpoint = getPIDSetpoint();
    dataLog[dataLogIndex].state = currentState;
    dataLogIndex++;
  }
}

DataPoint* getDataLog() {
  return dataLog;
}

int getDataLogIndex() {
  return dataLogIndex;
}

void resetDataLog() {
  dataLogIndex = 0;
}
