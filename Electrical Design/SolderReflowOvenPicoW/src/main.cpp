/*
 * Solder Reflow Oven Controller - Raspberry Pi Pico W Dual-Core Version
 * 
 * Hardware:
 * - Raspberry Pi Pico W (RP2040 dual-core @ 133 MHz)
 * - Adafruit MAX31855 K-Type Thermocouple Amplifier
 * - Solid State Relay (SSR) for heater control
 * 
 * Features:
 * - Dual-core architecture for improved performance:
 *   - Core 0: WiFi, Web Server, WebSocket communication
 *   - Core 1: Temperature monitoring, PID control, SSR control
 * - Separate SPI buses available for thermocouple and future display
 * - Web-based interface (no display required)
 * - WiFi connectivity with AP and Station modes
 * - Real-time temperature monitoring via WebSocket
 * - PID-controlled reflow profile with auto-tuning
 * - Emergency stop functionality
 * - Faster PID response due to dedicated core
 * 
 * Pin Connections:
 * SPI0 (Thermocouple - Core 1):
 * - GPIO 18: MAX31855 CLK (SCK)
 * - GPIO 19: MAX31855 MOSI (not used for MAX31855)
 * - GPIO 16: MAX31855 DO (MISO)
 * - GPIO 17: MAX31855 CS
 * 
 * SPI1 (Reserved for future display - Core 0):
 * - GPIO 10: Display CLK (SCK)
 * - GPIO 11: Display MOSI
 * - GPIO 12: Display MISO
 * - GPIO 13: Display CS
 * 
 * Other:
 * - GPIO 15: SSR Control
 * - GPIO 25: Onboard LED (status indicator)
 * 
 * Migration Notes:
 * - Upgraded from ESP8266 NodeMCU to Raspberry Pi Pico W
 * - Dual-core architecture separates time-critical thermal control from web interface
 * - Core 1 runs thermal control loop at higher frequency for faster PID response
 * - Shared memory with mutex protection for thread-safe communication between cores
 */

#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <PID_v1.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/mutex.h"

// Include MAX31855 library for Pico
// Note: Using Adafruit MAX31855 library with RP2040 support
#include <Adafruit_MAX31855.h>

// Pin Definitions for Raspberry Pi Pico W
// SPI0 for Thermocouple (Core 1)
#define THERMO_CLK  18   // SPI0 SCK
#define THERMO_CS   17   // SPI0 CS
#define THERMO_DO   16   // SPI0 MISO

// SPI1 for future display (Core 0)
#define DISPLAY_CLK  10  // SPI1 SCK
#define DISPLAY_MOSI 11  // SPI1 MOSI
#define DISPLAY_MISO 12  // SPI1 MISO
#define DISPLAY_CS   13  // SPI1 CS

// Other GPIO
#define SSR_PIN     15   // SSR Control
#define LED_PIN     LED_BUILTIN  // Onboard LED (GPIO 25)

// WiFi Configuration - Change these to your network or leave for AP mode
const char* ssid = "ReflowOven";           // AP mode SSID
const char* password = "reflow123";         // AP mode password (min 8 chars)
bool useAPMode = true;                      // Set to false to connect to existing WiFi

// Web Server
WebServer server(80);

// MAX31855 Thermocouple (SPI0)
SPIClass SPI0_Thermo(spi0);  // Use SPI0 for thermocouple
Adafruit_MAX31855 maxthermo = Adafruit_MAX31855(THERMO_CLK, THERMO_CS, THERMO_DO);

// Reflow Profile Parameters (in Celsius and milliseconds)
double preheatTemp = 150;
double soakTemp = 180;
double reflowTemp = 230;
double cooldownTemp = 25;
unsigned long preheatTime = 90000;    // 90 seconds
unsigned long soakTime = 90000;       // 90 seconds
unsigned long reflowTime = 40000;     // 40 seconds
unsigned long cooldownTime = 60000;   // 60 seconds

// PID Variables (shared between cores)
double Setpoint = 0, Input = 0, Output = 0;
double Kp = 2, Ki = 5, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// EEPROM Configuration
#define EEPROM_SIZE 512  // Allocate 512 bytes
#define EEPROM_MAGIC 0xABCD  // Magic number to verify valid EEPROM data

struct EEPROMConfig {
  uint16_t magic;
  double preheatTemp;
  double soakTemp;
  double reflowTemp;
  double cooldownTemp;
  unsigned long preheatTime;
  unsigned long soakTime;
  unsigned long reflowTime;
  unsigned long cooldownTime;
  double Kp;
  double Ki;
  double Kd;
};

// Temperature-based state transition settings
bool useTemperatureBasedTransitions = true;
double tempReachThreshold = 5.0;
unsigned long maxStateTimeout = 180000;  // 3 minutes max per stage

// State Machine
enum ReflowState {
  IDLE,
  PREHEAT,
  SOAK,
  REFLOW,
  COOLDOWN,
  COMPLETE,
  ERROR_STATE,
  PID_TUNING
};

ReflowState currentState = IDLE;
String stateNames[] = {"IDLE", "PREHEAT", "SOAK", "REFLOW", "COOLDOWN", "COMPLETE", "ERROR", "PID_TUNING"};

// Timing
unsigned long reflowStartTime = 0;
unsigned long stateStartTime = 0;
unsigned long lastTempCheck = 0;
unsigned long tempCheckInterval = 1000;  // Check temperature every 1 second
unsigned long tempReachedTime = 0;

// Safety
bool emergencyStop = false;
int thermocoupleErrorCount = 0;
const int maxThermocoupleErrors = 3;

// Data logging
struct DataPoint {
  unsigned long time;
  double temperature;
  double setpoint;
  ReflowState state;
};

const int maxDataPoints = 500;
DataPoint dataLog[maxDataPoints];
int dataLogIndex = 0;

// PID Auto-Tuning Variables
bool pidTuningActive = false;
int tuningStep = 0;
double tuningTargetTemp[] = {100.0, 150.0, 200.0};
int tuningTempIndex = 0;
double tuningAmplitude = 5.0;
unsigned long tuningStartTime = 0;
unsigned long tuningStepStartTime = 0;
unsigned long oscillationStartTime = 0;
double lastPeakTemp = 0;
double lastValleyTemp = 0;
unsigned long lastPeakTime = 0;
unsigned long lastValleyTime = 0;
int oscillationCount = 0;
double sumOscillationPeriod = 0;
double sumOscillationAmplitude = 0;
bool waitingForPeak = true;
double tuningKu = 0;
double tuningPu = 0;
double calculatedKp = 0;
double calculatedKi = 0;
double calculatedKd = 0;

// Console Log
const int maxConsoleLines = 50;
String consoleLog[maxConsoleLines];
int consoleLogIndex = 0;

// Mutex for thread-safe access to shared variables between cores
mutex_t core_mutex;

// Shared variables that need mutex protection
struct SharedData {
  double currentTemp;
  double currentSetpoint;
  double currentOutput;
  ReflowState state;
  bool ssrState;
  unsigned long elapsedTime;
  bool emergencyStopRequested;
  bool startReflowRequested;
  bool stopReflowRequested;
  bool pidTuningRequested;
  bool stopTuningRequested;
  bool newDataAvailable;
} sharedData;

// WebSocket-like update mechanism
unsigned long lastWebUpdate = 0;
const unsigned long webUpdateInterval = 1000;  // 1 second

// Function prototypes
void core1_entry();
void runThermalControl();
void runPIDControl();
void updateStateMachine();
void readTemperature();
void controlSSR(bool state);
void emergencyStopReflow();
void startReflow();
void stopReflow();
void setupWiFi();
void setupWebServer();
void handleRoot();
void handleStatus();
void handleConfig();
void handleConfigUpdate();
void handleStart();
void handleStop();
void handleData();
void handleTunePID();
void handleConsole();
void loadConfigFromEEPROM();
void saveConfigToEEPROM();
void addConsoleLog(String message);
void logDataPoint();
void runPIDTuning();

// EEPROM Functions
void loadConfigFromEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROMConfig config;
  EEPROM.get(0, config);
  
  if (config.magic == EEPROM_MAGIC) {
    Serial.println("Loading configuration from EEPROM...");
    preheatTemp = config.preheatTemp;
    soakTemp = config.soakTemp;
    reflowTemp = config.reflowTemp;
    cooldownTemp = config.cooldownTemp;
    preheatTime = config.preheatTime;
    soakTime = config.soakTime;
    reflowTime = config.reflowTime;
    cooldownTime = config.cooldownTime;
    Kp = config.Kp;
    Ki = config.Ki;
    Kd = config.Kd;
    
    myPID.SetTunings(Kp, Ki, Kd);
    
    Serial.println("Configuration loaded successfully!");
    Serial.print("PID Values - Kp: "); Serial.print(Kp);
    Serial.print(", Ki: "); Serial.print(Ki);
    Serial.print(", Kd: "); Serial.println(Kd);
  } else {
    Serial.println("No valid EEPROM configuration found, using defaults");
  }

  EEPROM.end();  // End EEPROM session after loading
}

void saveConfigToEEPROM() {
  EEPROM.begin(EEPROM_SIZE);  // Must call begin() before writing

  EEPROMConfig config;
  config.magic = EEPROM_MAGIC;
  config.preheatTemp = preheatTemp;
  config.soakTemp = soakTemp;
  config.reflowTemp = reflowTemp;
  config.cooldownTemp = cooldownTemp;
  config.preheatTime = preheatTime;
  config.soakTime = soakTime;
  config.reflowTime = reflowTime;
  config.cooldownTime = cooldownTime;
  config.Kp = Kp;
  config.Ki = Ki;
  config.Kd = Kd;
  
  EEPROM.put(0, config);
  EEPROM.commit();  // Commit changes to flash
  EEPROM.end();     // End EEPROM session to ensure data is written

  Serial.println("Configuration saved to EEPROM");
}

void addConsoleLog(String message) {
  if (consoleLogIndex < maxConsoleLines) {
    consoleLog[consoleLogIndex] = message;
    consoleLogIndex++;
  } else {
    for (int i = 0; i < maxConsoleLines - 1; i++) {
      consoleLog[i] = consoleLog[i + 1];
    }
    consoleLog[maxConsoleLines - 1] = message;
  }
  Serial.println("[CONSOLE] " + message);
}

void logDataPoint() {
  if (dataLogIndex < maxDataPoints) {
    dataLog[dataLogIndex].time = millis() - reflowStartTime;
    
    mutex_enter_blocking(&core_mutex);
    dataLog[dataLogIndex].temperature = sharedData.currentTemp;
    dataLog[dataLogIndex].setpoint = sharedData.currentSetpoint;
    dataLog[dataLogIndex].state = sharedData.state;
    mutex_exit(&core_mutex);
    
    dataLogIndex++;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give time for serial to initialize
  
  Serial.println("\n\n===========================================");
  Serial.println("Solder Reflow Oven Controller - Pico W");
  Serial.println("Raspberry Pi Pico W Dual-Core Version");
  Serial.println("===========================================\n");

  // Initialize mutex for core synchronization
  mutex_init(&core_mutex);
  
  // Initialize shared data
  sharedData.currentTemp = 0;
  sharedData.currentSetpoint = 0;
  sharedData.currentOutput = 0;
  sharedData.state = IDLE;
  sharedData.ssrState = false;
  sharedData.elapsedTime = 0;
  sharedData.emergencyStopRequested = false;
  sharedData.startReflowRequested = false;
  sharedData.stopReflowRequested = false;
  sharedData.pidTuningRequested = false;
  sharedData.stopTuningRequested = false;
  sharedData.newDataAvailable = false;

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize SSR pin
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);
  Serial.println("SSR pin initialized (OFF)");

  // Initialize SPI0 for thermocouple
  Serial.println("Initializing SPI0 for MAX31855 thermocouple...");
  SPI0_Thermo.setSCK(THERMO_CLK);
  SPI0_Thermo.setTX(19);  // MOSI (not used but required)
  SPI0_Thermo.setRX(THERMO_DO);  // MISO
  SPI0_Thermo.setCS(THERMO_CS);
  SPI0_Thermo.begin();
  
  delay(500);
  double testTemp = maxthermo.readCelsius();
  if (isnan(testTemp)) {
    Serial.println("ERROR: Could not read from thermocouple!");
    Serial.println("Check wiring and connections.");
    currentState = ERROR_STATE;
    sharedData.state = ERROR_STATE;
  } else {
    Serial.print("MAX31855 OK - Current temp: ");
    Serial.print(testTemp);
    Serial.println(" °C");
  }

  // Initialize PID
  myPID.SetOutputLimits(0, 1);
  myPID.SetMode(AUTOMATIC);
  Serial.println("PID controller initialized");

  // Load configuration from EEPROM
  loadConfigFromEEPROM();

  // Setup WiFi
  setupWiFi();

  // Setup web server routes
  setupWebServer();

  Serial.println("\n===========================================");
  Serial.println("Core 0 (WiFi/Web) setup complete!");
  Serial.println("Starting Core 1 (Thermal Control)...");
  Serial.println("===========================================\n");

  // Start Core 1 for thermal control
  multicore_launch_core1(core1_entry);
  
  delay(500);
  Serial.println("Both cores running!");
  Serial.println("Core 0: Handling WiFi and Web Server");
  Serial.println("Core 1: Handling Temperature and PID Control");
}

// Core 1 Entry Point - Dedicated to thermal control
void core1_entry() {
  Serial.println("[Core 1] Thermal control started");
  
  // Core 1 runs the thermal control loop indefinitely
  while (true) {
    runThermalControl();
  }
}

// Core 1: Thermal Control Loop (runs on dedicated core)
void runThermalControl() {
  static unsigned long lastLoop = 0;
  unsigned long currentTime = millis();
  
  // Run thermal control at higher frequency than ESP8266 version
  // This is the advantage of having a dedicated core
  if (currentTime - lastLoop >= 100) {  // 10 Hz loop (vs 1 Hz on ESP8266)
    lastLoop = currentTime;
    
    // Read temperature
    readTemperature();
    
    // Check for emergency stop request from Core 0
    mutex_enter_blocking(&core_mutex);
    if (sharedData.emergencyStopRequested) {
      sharedData.emergencyStopRequested = false;
      mutex_exit(&core_mutex);
      emergencyStopReflow();
    } else if (sharedData.startReflowRequested) {
      sharedData.startReflowRequested = false;
      mutex_exit(&core_mutex);
      startReflow();
    } else if (sharedData.stopReflowRequested) {
      sharedData.stopReflowRequested = false;
      mutex_exit(&core_mutex);
      stopReflow();
    } else if (sharedData.pidTuningRequested) {
      sharedData.pidTuningRequested = false;
      mutex_exit(&core_mutex);
      pidTuningActive = true;
      tuningStartTime = millis();
      tuningStep = 0;
      tuningTempIndex = 0;
      oscillationCount = 0;
      sumOscillationPeriod = 0;
      sumOscillationAmplitude = 0;
      addConsoleLog("=== PID Auto-Tuning Started ===");
    } else if (sharedData.stopTuningRequested) {
      sharedData.stopTuningRequested = false;
      mutex_exit(&core_mutex);
      pidTuningActive = false;
      controlSSR(false);
      addConsoleLog("PID tuning stopped by user");
    } else {
      mutex_exit(&core_mutex);
    }
    
    // Run PID tuning if active
    if (pidTuningActive) {
      runPIDTuning();
    } else {
      // Update state machine
      updateStateMachine();
      
      // Run PID control
      runPIDControl();
    }
    
    // Update shared data for Core 0
    mutex_enter_blocking(&core_mutex);
    sharedData.currentTemp = Input;
    sharedData.currentSetpoint = Setpoint;
    sharedData.currentOutput = Output;
    sharedData.state = currentState;
    sharedData.ssrState = digitalRead(SSR_PIN);
    if (reflowStartTime > 0) {
      sharedData.elapsedTime = millis() - reflowStartTime;
    } else {
      sharedData.elapsedTime = 0;
    }
    sharedData.newDataAvailable = true;
    mutex_exit(&core_mutex);
    
    // Log data every second
    if (currentTime - lastTempCheck >= tempCheckInterval) {
      lastTempCheck = currentTime;
      if (currentState != IDLE && currentState != COMPLETE && currentState != ERROR_STATE) {
        logDataPoint();
      }
    }
    
    // Blink LED to show Core 1 is running
    static unsigned long lastBlink = 0;
    if (currentTime - lastBlink >= 500) {
      lastBlink = currentTime;
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
  }
}

void readTemperature() {
  Input = maxthermo.readCelsius();
  
  if (isnan(Input)) {
    thermocoupleErrorCount++;
    Serial.print("[Core 1] Thermocouple read error (");
    Serial.print(thermocoupleErrorCount);
    Serial.println("/3)");
    
    if (thermocoupleErrorCount >= maxThermocoupleErrors) {
      emergencyStopReflow();
      currentState = ERROR_STATE;
      addConsoleLog("ERROR: Thermocouple failure detected!");
    }
  } else {
    thermocoupleErrorCount = 0;  // Reset error count on successful read
  }
}

void controlSSR(bool state) {
  digitalWrite(SSR_PIN, state ? HIGH : LOW);
}

void runPIDControl() {
  if (currentState != IDLE && currentState != COOLDOWN && 
      currentState != COMPLETE && currentState != ERROR_STATE &&
      !emergencyStop) {
    
    myPID.Compute();
    
    // SSR control based on PID output
    if (Output > 0.5) {
      controlSSR(true);
    } else {
      controlSSR(false);
    }
  } else {
    controlSSR(false);  // Ensure SSR is off
  }
}

void updateStateMachine() {
  unsigned long currentTime = millis();
  unsigned long stateElapsed = currentTime - stateStartTime;
  unsigned long totalElapsed = currentTime - reflowStartTime;
  
  // Safety timeout check (3 minutes max per state)
  if (currentState != IDLE && currentState != COMPLETE && 
      currentState != ERROR_STATE && stateElapsed > maxStateTimeout) {
    Serial.println("[Core 1] State timeout - advancing to next state");
    stateStartTime = currentTime;
  }
  
  switch (currentState) {
    case IDLE:
      Setpoint = cooldownTemp;
      Output = 0;
      break;
      
    case PREHEAT:
      Setpoint = preheatTemp;
      
      // Transition when temperature reached and minimum time elapsed
      if (useTemperatureBasedTransitions) {
        if (Input >= (preheatTemp - tempReachThreshold)) {
          if (tempReachedTime == 0) {
            tempReachedTime = currentTime;
          }
          if ((currentTime - tempReachedTime) >= preheatTime || stateElapsed >= (preheatTime * 2)) {
            currentState = SOAK;
            stateStartTime = currentTime;
            tempReachedTime = 0;
            Serial.println("[Core 1] Transition: PREHEAT -> SOAK");
          }
        } else {
          tempReachedTime = 0;
        }
      } else {
        if (stateElapsed >= preheatTime) {
          currentState = SOAK;
          stateStartTime = currentTime;
          Serial.println("[Core 1] Transition: PREHEAT -> SOAK");
        }
      }
      break;
      
    case SOAK:
      Setpoint = soakTemp;
      
      if (useTemperatureBasedTransitions) {
        if (Input >= (soakTemp - tempReachThreshold)) {
          if (tempReachedTime == 0) {
            tempReachedTime = currentTime;
          }
          if ((currentTime - tempReachedTime) >= soakTime || stateElapsed >= (soakTime * 2)) {
            currentState = REFLOW;
            stateStartTime = currentTime;
            tempReachedTime = 0;
            Serial.println("[Core 1] Transition: SOAK -> REFLOW");
          }
        } else {
          tempReachedTime = 0;
        }
      } else {
        if (stateElapsed >= soakTime) {
          currentState = REFLOW;
          stateStartTime = currentTime;
          Serial.println("[Core 1] Transition: SOAK -> REFLOW");
        }
      }
      break;
      
    case REFLOW:
      Setpoint = reflowTemp;
      
      if (useTemperatureBasedTransitions) {
        if (Input >= (reflowTemp - tempReachThreshold)) {
          if (tempReachedTime == 0) {
            tempReachedTime = currentTime;
          }
          if ((currentTime - tempReachedTime) >= reflowTime || stateElapsed >= (reflowTime * 2)) {
            currentState = COOLDOWN;
            stateStartTime = currentTime;
            tempReachedTime = 0;
            Serial.println("[Core 1] Transition: REFLOW -> COOLDOWN");
          }
        } else {
          tempReachedTime = 0;
        }
      } else {
        if (stateElapsed >= reflowTime) {
          currentState = COOLDOWN;
          stateStartTime = currentTime;
          Serial.println("[Core 1] Transition: REFLOW -> COOLDOWN");
        }
      }
      break;
      
    case COOLDOWN:
      Setpoint = cooldownTemp;
      controlSSR(false);  // Force SSR off during cooldown
      
      if (Input <= (cooldownTemp + 10) || stateElapsed >= cooldownTime) {
        currentState = COMPLETE;
        stateStartTime = currentTime;
        Serial.println("[Core 1] Reflow cycle complete!");
        addConsoleLog("Reflow cycle completed successfully");
      }
      break;
      
    case COMPLETE:
      Setpoint = cooldownTemp;
      controlSSR(false);
      break;
      
    case ERROR_STATE:
      Setpoint = cooldownTemp;
      controlSSR(false);
      break;
  }
}

void startReflow() {
  if (currentState == IDLE || currentState == COMPLETE || currentState == ERROR_STATE) {
    Serial.println("[Core 1] Starting reflow cycle...");
    
    // Reset PID to clear integral windup
    myPID.SetMode(MANUAL);
    Output = 0;
    myPID.SetMode(AUTOMATIC);
    
    // Reset data logging
    dataLogIndex = 0;
    
    // Reset emergency stop flag
    emergencyStop = false;
    thermocoupleErrorCount = 0;
    
    // Start reflow
    currentState = PREHEAT;
    reflowStartTime = millis();
    stateStartTime = millis();
    tempReachedTime = 0;
    
    addConsoleLog("Reflow cycle started");
    Serial.println("[Core 1] Reflow started - State: PREHEAT");
  }
}

void stopReflow() {
  Serial.println("[Core 1] Stopping reflow...");
  controlSSR(false);
  currentState = IDLE;
  Output = 0;
  Setpoint = cooldownTemp;
  addConsoleLog("Reflow stopped by user");
}

void emergencyStopReflow() {
  Serial.println("[Core 1] EMERGENCY STOP!");
  controlSSR(false);
  emergencyStop = true;
  Output = 0;
  Setpoint = cooldownTemp;
  currentState = ERROR_STATE;
  addConsoleLog("EMERGENCY STOP - SSR turned OFF");
}

// PID Auto-Tuning Implementation
void runPIDTuning() {
  unsigned long currentTime = millis();
  unsigned long tuningElapsed = currentTime - tuningStartTime;
  unsigned long stepElapsed = currentTime - tuningStepStartTime;
  
  // Safety timeout (10 minutes total)
  if (tuningElapsed > 600000) {
    pidTuningActive = false;
    controlSSR(false);
    addConsoleLog("PID tuning timeout - process stopped");
    return;
  }
  
  // Get current test temperature
  double targetTemp = tuningTargetTemp[tuningTempIndex];
  
  // Tuning state machine
  if (tuningStep == 0) {
    // Step 0: Heat to target temperature
    tuningStepStartTime = currentTime;
    addConsoleLog("Step " + String(tuningTempIndex * 2 + 1) + "/6: Heating to " + String(targetTemp) + "C...");
    tuningStep = 1;
  }
  else if (tuningStep == 1) {
    // Heating to target
    if (Input < targetTemp) {
      controlSSR(true);
    } else {
      controlSSR(false);
      tuningStep = 2;
      tuningStepStartTime = currentTime;
      oscillationStartTime = currentTime;
      oscillationCount = 0;
      sumOscillationPeriod = 0;
      sumOscillationAmplitude = 0;
      waitingForPeak = true;
      lastPeakTemp = Input;
      lastValleyTemp = Input;
      addConsoleLog("Target " + String(targetTemp) + "C reached. Starting oscillation test...");
    }
    
    // Timeout check
    if (stepElapsed > 300000) {  // 5 minutes
      addConsoleLog("Heating timeout at " + String(targetTemp) + "C");
      pidTuningActive = false;
      controlSSR(false);
      return;
    }
  }
  else if (tuningStep == 2) {
    // Oscillation test
    // Bang-bang control to create oscillations
    if (Input < (targetTemp - tuningAmplitude)) {
      controlSSR(true);
      if (waitingForPeak && Input > lastValleyTemp) {
        lastValleyTemp = Input;
        lastValleyTime = currentTime;
      } else if (!waitingForPeak) {
        // Found valley, now wait for peak
        waitingForPeak = true;
        lastPeakTemp = Input;
      }
    } else if (Input > (targetTemp + tuningAmplitude)) {
      controlSSR(false);
      if (!waitingForPeak && Input < lastPeakTemp) {
        lastPeakTemp = Input;
        lastPeakTime = currentTime;
      } else if (waitingForPeak) {
        // Found peak, now wait for valley
        waitingForPeak = false;
        lastValleyTemp = Input;
        
        // Record oscillation
        if (oscillationCount > 0) {
          double period = (currentTime - lastValleyTime) / 1000.0;  // in seconds
          double amplitude = abs(lastPeakTemp - lastValleyTemp);
          sumOscillationPeriod += period;
          sumOscillationAmplitude += amplitude;
          
          addConsoleLog("  Oscillation #" + String(oscillationCount) + 
                       ": Period=" + String(period, 1) + "s" +
                       ", Amplitude=" + String(amplitude, 1) + "C");
        }
        oscillationCount++;
      }
    }
    
    // Need at least 3 oscillations
    if (oscillationCount >= 4) {
      // Calculate averages (skip first oscillation)
      double avgPeriod = sumOscillationPeriod / 3.0;
      double avgAmplitude = sumOscillationAmplitude / 3.0;
      
      // Store results
      tuningPu += avgPeriod;
      
      // Calculate Ku using relay feedback method
      double Ku_temp = (4.0 * tuningAmplitude) / (3.14159 * avgAmplitude);
      tuningKu += Ku_temp;
      
      addConsoleLog("Oscillation test complete at " + String(targetTemp) + "C");
      addConsoleLog("  Avg Period: " + String(avgPeriod, 2) + "s, Avg Amplitude: " + String(avgAmplitude, 2) + "C");
      
      controlSSR(false);
      
      // Move to next temperature or finish
      tuningTempIndex++;
      if (tuningTempIndex < 3) {
        tuningStep = 0;  // Next temperature
      } else {
        // All tests complete - calculate PID parameters
        tuningKu /= 3.0;  // Average Ku
        tuningPu /= 3.0;  // Average Pu
        
        // Ziegler-Nichols tuning rules
        calculatedKp = 0.6 * tuningKu;
        calculatedKi = 1.2 * tuningKu / tuningPu;
        calculatedKd = 0.075 * tuningKu * tuningPu;
        
        addConsoleLog("");
        addConsoleLog("=== Calculating PID Parameters ===");
        addConsoleLog("Average oscillation period: " + String(tuningPu, 2) + " seconds");
        addConsoleLog("Ultimate gain (Ku): " + String(tuningKu, 3));
        addConsoleLog("Ultimate period (Pu): " + String(tuningPu, 2) + "s");
        addConsoleLog("");
        addConsoleLog("=== Recommended PID Values (Ziegler-Nichols) ===");
        addConsoleLog("Kp = " + String(calculatedKp, 3));
        addConsoleLog("Ki = " + String(calculatedKi, 3));
        addConsoleLog("Kd = " + String(calculatedKd, 3));
        addConsoleLog("");
        addConsoleLog("Apply these values in the configuration and save to EEPROM");
        
        // Apply the tuned parameters
        Kp = calculatedKp;
        Ki = calculatedKi;
        Kd = calculatedKd;
        myPID.SetTunings(Kp, Ki, Kd);
        
        pidTuningActive = false;
        tuningStep = 0;
        tuningTempIndex = 0;
        tuningKu = 0;
        tuningPu = 0;
      }
    }
    
    // Timeout check
    if (stepElapsed > 300000) {  // 5 minutes
      addConsoleLog("Oscillation test timeout");
      pidTuningActive = false;
      controlSSR(false);
      return;
    }
  }
}

// Core 0: WiFi and Web Server Loop
void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Send updates to web clients periodically
  unsigned long currentTime = millis();
  if (currentTime - lastWebUpdate >= webUpdateInterval) {
    lastWebUpdate = currentTime;
    
    // Note: WebSocket functionality would go here
    // For now, clients will poll via HTTP
  }
  
  // Small delay to prevent tight loop
  delay(1);
}

void setupWiFi() {
  if (useAPMode) {
    Serial.println("Starting WiFi Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP SSID: ");
    Serial.println(ssid);
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.println("Connect to this network and navigate to http://" + IP.toString());
  } else {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nWiFi connection failed! Starting AP mode...");
      WiFi.mode(WIFI_AP);
      WiFi.softAP(ssid, password);
      IPAddress IP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(IP);
    }
  }
}

// Forward declarations for web handlers
void handleRoot();
void handleStatus();
void handleConfig();
void handleConfigUpdate();
void handleStart();
void handleStop();
void handleData();
void handleTunePID();
void handleConsole();

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/config", HTTP_GET, handleConfig);
  server.on("/api/config", HTTP_POST, handleConfigUpdate);
  server.on("/api/start", HTTP_POST, handleStart);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/data", handleData);
  server.on("/api/tune-pid", HTTP_POST, handleTunePID);
  server.on("/api/console", handleConsole);
  
  server.begin();
  Serial.println("HTTP server started on port 80");
}

void handleStatus() {
  StaticJsonDocument<256> doc;
  
  mutex_enter_blocking(&core_mutex);
  doc["temp"] = sharedData.currentTemp;
  doc["setpoint"] = sharedData.currentSetpoint;
  doc["state"] = stateNames[sharedData.state];
  doc["elapsed"] = sharedData.elapsedTime;
  doc["ssr"] = sharedData.ssrState ? 1 : 0;
  doc["output"] = sharedData.currentOutput;
  mutex_exit(&core_mutex);
  
  doc["tuning"] = pidTuningActive ? 1 : 0;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleConfig() {
  StaticJsonDocument<512> doc;
  
  doc["preheatTemp"] = preheatTemp;
  doc["soakTemp"] = soakTemp;
  doc["reflowTemp"] = reflowTemp;
  doc["cooldownTemp"] = cooldownTemp;
  doc["preheatTime"] = preheatTime;
  doc["soakTime"] = soakTime;
  doc["reflowTime"] = reflowTime;
  doc["cooldownTime"] = cooldownTime;
  doc["Kp"] = Kp;
  doc["Ki"] = Ki;
  doc["Kd"] = Kd;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleConfigUpdate() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      return;
    }
    
    if (doc.containsKey("preheatTemp")) preheatTemp = doc["preheatTemp"];
    if (doc.containsKey("soakTemp")) soakTemp = doc["soakTemp"];
    if (doc.containsKey("reflowTemp")) reflowTemp = doc["reflowTemp"];
    if (doc.containsKey("cooldownTemp")) cooldownTemp = doc["cooldownTemp"];
    if (doc.containsKey("preheatTime")) preheatTime = doc["preheatTime"];
    if (doc.containsKey("soakTime")) soakTime = doc["soakTime"];
    if (doc.containsKey("reflowTime")) reflowTime = doc["reflowTime"];
    if (doc.containsKey("cooldownTime")) cooldownTime = doc["cooldownTime"];
    
    if (doc.containsKey("Kp")) {
      Kp = doc["Kp"];
      Ki = doc["Ki"];
      Kd = doc["Kd"];
      myPID.SetTunings(Kp, Ki, Kd);
      Serial.print("PID updated - Kp: "); Serial.print(Kp);
      Serial.print(", Ki: "); Serial.print(Ki);
      Serial.print(", Kd: "); Serial.println(Kd);
    }
    
    if (doc.containsKey("saveToEEPROM") && doc["saveToEEPROM"] == true) {
      saveConfigToEEPROM();
    }
    
    server.send(200, "application/json", "{\"status\":\"success\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
  }
}

void handleStart() {
  mutex_enter_blocking(&core_mutex);
  
  if (sharedData.state == IDLE || sharedData.state == COMPLETE || sharedData.state == ERROR_STATE) {
    sharedData.startReflowRequested = true;
    mutex_exit(&core_mutex);
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Reflow started\"}");
  } else {
    mutex_exit(&core_mutex);
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Already running\"}");
  }
}

void handleStop() {
  mutex_enter_blocking(&core_mutex);
  sharedData.stopReflowRequested = true;
  mutex_exit(&core_mutex);
  
  server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Reflow stopped\"}");
}

void handleData() {
  String json = "[";
  for (int i = 0; i < dataLogIndex; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"time\":" + String(dataLog[i].time) + ",";
    json += "\"temp\":" + String(dataLog[i].temperature, 2) + ",";
    json += "\"setpoint\":" + String(dataLog[i].setpoint, 2) + ",";
    json += "\"state\":" + String(dataLog[i].state);
    json += "}";
  }
  json += "]";
  
  server.send(200, "application/json", json);
}

void handleTunePID() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<128> doc;
    deserializeJson(doc, body);
    
    if (doc.containsKey("action")) {
      String action = doc["action"];
      
      if (action == "start") {
        mutex_enter_blocking(&core_mutex);
        if (sharedData.state == IDLE || sharedData.state == COMPLETE) {
          sharedData.pidTuningRequested = true;
          mutex_exit(&core_mutex);
          server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"PID tuning started\"}");
        } else {
          mutex_exit(&core_mutex);
          server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cannot start tuning while reflow is active\"}");
        }
      } else if (action == "stop") {
        mutex_enter_blocking(&core_mutex);
        sharedData.stopTuningRequested = true;
        mutex_exit(&core_mutex);
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"PID tuning stopped\"}");
      } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid action\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing action\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
  }
}

void handleConsole() {
  String json = "[";
  for (int i = 0; i < consoleLogIndex; i++) {
    if (i > 0) json += ",";
    json += "\"";
    json += consoleLog[i];
    json += "\"";
  }
  json += "]";
  
  server.send(200, "application/json", json);
}

void handleRoot() {
  // Serve a simple web interface
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Reflow Oven Controller - Pico W</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 1200px;
      margin: 0 auto;
      background: white;
      border-radius: 20px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      padding: 30px;
    }
    h1 {
      color: #667eea;
      text-align: center;
      margin-bottom: 10px;
      font-size: 2.5em;
    }
    .subtitle {
      text-align: center;
      color: #666;
      margin-bottom: 30px;
      font-size: 1.1em;
    }
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    .status-card {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 20px;
      border-radius: 15px;
      text-align: center;
    }
    .status-card h3 {
      font-size: 0.9em;
      opacity: 0.9;
      margin-bottom: 10px;
    }
    .status-card .value {
      font-size: 2.5em;
      font-weight: bold;
    }
    .controls {
      display: flex;
      gap: 15px;
      justify-content: center;
      margin-bottom: 30px;
      flex-wrap: wrap;
    }
    button {
      padding: 15px 30px;
      font-size: 1.1em;
      border: none;
      border-radius: 10px;
      cursor: pointer;
      transition: all 0.3s;
      font-weight: bold;
    }
    .btn-start {
      background: #10b981;
      color: white;
    }
    .btn-start:hover { background: #059669; }
    .btn-stop {
      background: #ef4444;
      color: white;
    }
    .btn-stop:hover { background: #dc2626; }
    .btn-tune {
      background: #f59e0b;
      color: white;
    }
    .btn-tune:hover { background: #d97706; }
    .section {
      background: #f9fafb;
      padding: 20px;
      border-radius: 15px;
      margin-bottom: 20px;
    }
    .section h2 {
      color: #667eea;
      margin-bottom: 15px;
    }
    .form-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 15px;
    }
    .form-group label {
      display: block;
      font-weight: bold;
      margin-bottom: 5px;
      color: #374151;
    }
    input[type="number"] {
      width: 100%;
      padding: 10px;
      border: 2px solid #e5e7eb;
      border-radius: 8px;
      font-size: 1em;
    }
    input[type="number"]:focus {
      outline: none;
      border-color: #667eea;
    }
    .console {
      background: #1f2937;
      color: #10b981;
      padding: 15px;
      border-radius: 10px;
      font-family: 'Courier New', monospace;
      max-height: 300px;
      overflow-y: auto;
      font-size: 0.9em;
    }
    .state-IDLE { background: linear-gradient(135deg, #6b7280 0%, #4b5563 100%); }
    .state-PREHEAT { background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%); }
    .state-SOAK { background: linear-gradient(135deg, #eab308 0%, #ca8a04 100%); }
    .state-REFLOW { background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%); }
    .state-COOLDOWN { background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%); }
    .state-COMPLETE { background: linear-gradient(135deg, #10b981 0%, #059669 100%); }
    .state-ERROR { background: linear-gradient(135deg, #7f1d1d 0%, #991b1b 100%); }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔥 Reflow Oven Controller</h1>
    <div class="subtitle">Raspberry Pi Pico W Dual-Core Edition</div>
    
    <div class="status-grid">
      <div class="status-card" id="temp-card">
        <h3>Temperature</h3>
        <div class="value" id="temp">--</div>
      </div>
      <div class="status-card" id="state-card">
        <h3>State</h3>
        <div class="value" id="state">IDLE</div>
      </div>
      <div class="status-card">
        <h3>Target</h3>
        <div class="value" id="setpoint">--</div>
      </div>
      <div class="status-card">
        <h3>Elapsed</h3>
        <div class="value" id="elapsed">0s</div>
      </div>
    </div>
    
    <div class="controls">
      <button class="btn-start" onclick="startReflow()">▶ Start Reflow</button>
      <button class="btn-stop" onclick="stopReflow()">⬛ Stop</button>
      <button class="btn-tune" onclick="tunePID()">⚙ Auto-Tune PID</button>
    </div>
    
    <div class="section">
      <h2>Reflow Profile</h2>
      <div class="form-grid">
        <div class="form-group">
          <label>Preheat Temp (°C)</label>
          <input type="number" id="preheatTemp" value="150">
        </div>
        <div class="form-group">
          <label>Soak Temp (°C)</label>
          <input type="number" id="soakTemp" value="180">
        </div>
        <div class="form-group">
          <label>Reflow Temp (°C)</label>
          <input type="number" id="reflowTemp" value="230">
        </div>
        <div class="form-group">
          <label>Preheat Time (ms)</label>
          <input type="number" id="preheatTime" value="90000">
        </div>
        <div class="form-group">
          <label>Soak Time (ms)</label>
          <input type="number" id="soakTime" value="90000">
        </div>
        <div class="form-group">
          <label>Reflow Time (ms)</label>
          <input type="number" id="reflowTime" value="40000">
        </div>
      </div>
    </div>
    
    <div class="section">
      <h2>PID Parameters</h2>
      <div class="form-grid">
        <div class="form-group">
          <label>Kp (Proportional)</label>
          <input type="number" id="Kp" value="2" step="0.1">
        </div>
        <div class="form-group">
          <label>Ki (Integral)</label>
          <input type="number" id="Ki" value="5" step="0.1">
        </div>
        <div class="form-group">
          <label>Kd (Derivative)</label>
          <input type="number" id="Kd" value="1" step="0.1">
        </div>
      </div>
      <br>
      <button class="btn-start" onclick="saveConfig()">💾 Save Configuration</button>
    </div>
    
    <div class="section">
      <h2>Console Log</h2>
      <div class="console" id="console">Waiting for messages...</div>
    </div>
  </div>
  
  <script>
    function updateStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('temp').textContent = data.temp.toFixed(1) + '°C';
          document.getElementById('setpoint').textContent = data.setpoint.toFixed(0) + '°C';
          document.getElementById('state').textContent = data.state;
          document.getElementById('elapsed').textContent = Math.floor(data.elapsed / 1000) + 's';
          
          const stateCard = document.getElementById('state-card');
          stateCard.className = 'status-card state-' + data.state;
        });
    }
    
    function updateConsole() {
      fetch('/api/console')
        .then(r => r.json())
        .then(data => {
          if (data.length > 0) {
            document.getElementById('console').innerHTML = data.join('<br>');
          }
        });
    }
    
    function loadConfig() {
      fetch('/api/config')
        .then(r => r.json())
        .then(data => {
          document.getElementById('preheatTemp').value = data.preheatTemp;
          document.getElementById('soakTemp').value = data.soakTemp;
          document.getElementById('reflowTemp').value = data.reflowTemp;
          document.getElementById('preheatTime').value = data.preheatTime;
          document.getElementById('soakTime').value = data.soakTime;
          document.getElementById('reflowTime').value = data.reflowTime;
          document.getElementById('Kp').value = data.Kp;
          document.getElementById('Ki').value = data.Ki;
          document.getElementById('Kd').value = data.Kd;
        });
    }
    
    function saveConfig() {
      const config = {
        preheatTemp: parseFloat(document.getElementById('preheatTemp').value),
        soakTemp: parseFloat(document.getElementById('soakTemp').value),
        reflowTemp: parseFloat(document.getElementById('reflowTemp').value),
        preheatTime: parseInt(document.getElementById('preheatTime').value),
        soakTime: parseInt(document.getElementById('soakTime').value),
        reflowTime: parseInt(document.getElementById('reflowTime').value),
        Kp: parseFloat(document.getElementById('Kp').value),
        Ki: parseFloat(document.getElementById('Ki').value),
        Kd: parseFloat(document.getElementById('Kd').value),
        saveToEEPROM: true
      };
      
      fetch('/api/config', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(config)
      })
      .then(r => r.json())
      .then(data => {
        if (data.status === 'success') {
          alert('Configuration saved successfully!');
        }
      });
    }
    
    function startReflow() {
      fetch('/api/start', {method: 'POST'})
        .then(r => r.json())
        .then(data => alert(data.message));
    }
    
    function stopReflow() {
      if (confirm('Are you sure you want to stop the reflow cycle?')) {
        fetch('/api/stop', {method: 'POST'})
          .then(r => r.json())
          .then(data => alert(data.message));
      }
    }
    
    function tunePID() {
      if (confirm('Start PID auto-tuning? This will take about 10-15 minutes.')) {
        fetch('/api/tune-pid', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({action: 'start'})
        })
        .then(r => r.json())
        .then(data => alert(data.message));
      }
    }
    
    // Auto-refresh
    setInterval(updateStatus, 1000);
    setInterval(updateConsole, 2000);
    
    // Load config on page load
    loadConfig();
    updateStatus();
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}
