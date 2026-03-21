/*
 * Solder Reflow Oven Controller - Raspberry Pi Pico W Version
 * REFACTORED - Modular Architecture
 *
 * Hardware:
 * - Raspberry Pi Pico W (RP2040 + CYW43439)
 * - Adafruit MAX31855 K-Type Thermocouple Amplifier
 * - Solid State Relay (SSR) for heater control
 *
 * Features:
 * - Web-based interface (no display required)
 * - WiFi connectivity with AP and Station modes
 * - Real-time temperature monitoring via WebSocket
 * - Dual-core PID control (10 Hz update rate)
 * - Emergency stop functionality
 * - Automatic PID tuning
 */

#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "temperature.h"
#include "pid_controller.h"
#include "led_control.h"
#include "storage.h"
#include "presets.h"
#include "wifi_setup.h"
#include "web_server.h"
#include "state_machine.h"
#include "ota.h"

// Timing variables
unsigned long lastTempCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);  // Wait for serial connection

  Serial.println("\n\n=================================");
  Serial.println("Solder Reflow Oven Controller");
  Serial.println("Pico W + MAX31855 + Web Interface");
  Serial.println("MODULAR ARCHITECTURE");
  Serial.println("=================================\n");

  // Initialize LED
  initLED();

  // Initialize SSR pin
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);
  Serial.println("SSR pin initialized (OFF)");

  // Initialize temperature sensor
  initTemperatureSensor();

  // Initialize PID controller
  initPIDController();

  // Initialize storage and load configuration
  initStorage();
  loadConfigFromEEPROM();

  // Initialize presets
  initPresets();

  // Re-stabilize MAX31855 after LittleFS mount
  Serial.println("Re-stabilizing MAX31855 after LittleFS mount...");
  delay(200);
  Adafruit_MAX31855* thermo = getThermocoupleInstance();
  for (int i = 0; i < 3; i++) {
    thermo->readCelsius();
    delay(100);
  }
  Serial.println("MAX31855 re-stabilized");

  // Setup WiFi
  setupWiFi();

  // Initialize OTA (after WiFi is connected)
  if (!isCaptivePortalActive()) {
    initOTA();
  }

  // Set LED to solid on once WiFi is connected
  setLED(true);

  // Setup web server
  setupWebServer();

  // Initialize state machine
  initStateMachine();

  Serial.println("\n=================================");
  Serial.println("Setup complete!");
  Serial.println("LED Status Indicators:");
  Serial.println("  Solid ON    = Connected & Idle");
  Serial.println("  Fast Blink  = Reflow Active (4 Hz)");
  Serial.println("  Slow Blink  = Reflow Complete (1 Hz)");
  Serial.println("  Medium Blink = ERROR State (2 Hz)");
  Serial.println("=================================\n");
}

void loop() {
  // Process ArduinoOTA requests
  processOTA();

  // Process DNS queries for captive portal
  processDNSRequests();

  // Handle web server clients
  processWebServerRequests();

  // Update LED status indicator
  updateLED();

  // Read temperature periodically
  unsigned long currentTime = millis();
  if (currentTime - lastTempCheck >= TEMP_CHECK_INTERVAL) {
    lastTempCheck = currentTime;
    readTemperature();

    // Update state machine based on current state
    ReflowState state = getCurrentState();
    if (state == PID_TUNING) {
      updatePIDTuningStateMachine();
    } else if (state != IDLE && state != COMPLETE && state != ERROR_STATE) {
      updateReflowStateMachine();
    }

    // Log data point
    logDataPoint();
  }
}

