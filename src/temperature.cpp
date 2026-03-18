/*
 * Temperature Sensor Implementation
 */

#include "temperature.h"
#include "pins.h"
#include "config.h"

// Forward declare console log function
extern void addConsoleLog(String message);
extern void emergencyStopReflow();
extern ReflowState currentState;

static Adafruit_MAX31855 maxthermo(THERMO_CLK, THERMO_CS, THERMO_DO);
static double Input = 0;
static int thermocoupleErrorCount = 0;

void initTemperatureSensor() {
  Serial.println("Initializing MAX31855 thermocouple...");

  // CRITICAL FOR PICO W: Explicitly configure GPIO pins
  pinMode(THERMO_CLK, OUTPUT);
  pinMode(THERMO_CS, OUTPUT);
  pinMode(THERMO_DO, INPUT);
  digitalWrite(THERMO_CS, HIGH);  // CS starts high (inactive)
  Serial.println("MAX31855 GPIO pins configured");

  delay(50);

  if (!maxthermo.begin()) {
    Serial.println("ERROR: Failed to initialize MAX31855!");
    Serial.println("Check wiring: GPIO 2->CLK, 3->CS, 4->DO, 3.3V->VCC, GND->GND");
    currentState = ERROR_STATE;
  } else {
    Serial.println("MAX31855 initialized successfully");

    // MAX31855 needs warm-up time
    Serial.println("Waiting for MAX31855 to stabilize (500ms)...");
    delay(500);

    // First read - discard
    maxthermo.readCelsius();
    delay(100);

    // Second read - actual test
    double testTemp = maxthermo.readCelsius();
    if (isnan(testTemp)) {
      Serial.println("WARNING: Initial temperature read returned NaN");
      Serial.println("Checking for faults...");
      uint8_t error = maxthermo.readError();
      if (error & 0x01) Serial.println("  - Thermocouple open circuit (not connected)");
      if (error & 0x02) Serial.println("  - Thermocouple short to GND (may be false positive)");
      if (error & 0x04) Serial.println("  - Thermocouple short to VCC (may be false positive)");
      if (error == 0) Serial.println("  - No fault detected, sensor may need more warm-up time");
      Serial.println("Will retry during operation...");
    } else {
      Serial.print("MAX31855 OK - Current temp: ");
      Serial.print(testTemp);
      Serial.println(" °C");
    }
  }
}

void readTemperature() {
  // MAX31855 requires minimum 220ms for full conversion cycle
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastReadTime < 250) {
    return;  // Too soon, use previous value
  }
  lastReadTime = currentTime;

  double newTemp = maxthermo.readCelsius();

  if (isnan(newTemp)) {
    // Retry once to distinguish real faults from timing glitches
    delay(50);
    newTemp = maxthermo.readCelsius();

    if (!isnan(newTemp)) {
      // Retry succeeded - transient glitch
      Serial.println("WARNING: Transient thermocouple read error (recovered - SPI glitch)");
      addConsoleLog("WARNING: Transient thermocouple error (recovered - SPI glitch)");
      Input = newTemp;
      return;
    }

    // Genuine error
    thermocoupleErrorCount++;

    uint8_t error = maxthermo.readError();
    Serial.print("WARNING: Thermocouple read error! Error code: 0x");
    Serial.print(error, HEX);
    Serial.print(" - ");

    if (error & 0x01) {
      Serial.println("Thermocouple OPEN (not connected or broken)");
    } else if (error & 0x02) {
      Serial.println("Thermocouple SHORT to GND (may be false - check SPI timing)");
    } else if (error & 0x04) {
      Serial.println("Thermocouple SHORT to VCC (may be false - check SPI timing)");
    } else if (error == 0) {
      Serial.println("No fault bits set - possible SPI communication issue");
    } else {
      Serial.println("Multiple faults detected");
    }

    if (thermocoupleErrorCount == 1) {
      addConsoleLog("WARNING: Thermocouple read error!");
      if (error & 0x01) {
        addConsoleLog("  Error: Thermocouple OPEN (not connected)");
      } else if (error & 0x02) {
        addConsoleLog("  Error: SHORT to GND (SPI timing issue?)");
      } else if (error & 0x04) {
        addConsoleLog("  Error: SHORT to VCC (SPI timing issue?)");
      } else if (error == 0) {
        addConsoleLog("  Error: SPI communication problem");
        addConsoleLog("  Verify GPIO pins: 2->CLK, 3->CS, 4->DO");
      }
    }

    if (thermocoupleErrorCount >= MAX_THERMOCOUPLE_ERRORS) {
      Serial.println("CRITICAL: Multiple thermocouple errors - EMERGENCY STOP!");
      addConsoleLog("CRITICAL: Multiple thermocouple errors!");
      addConsoleLog("If multimeter shows no short, this is SPI timing issue");
      addConsoleLog("Check MAX31855 wiring:");
      addConsoleLog("  GPIO 2 -> CLK");
      addConsoleLog("  GPIO 3 -> CS");
      addConsoleLog("  GPIO 4 -> DO (MISO)");
      addConsoleLog("  3.3V -> VCC, GND -> GND");
      if (error & 0x01) {
        addConsoleLog("FAULT: Thermocouple OPEN - Is it plugged in?");
      }
      emergencyStopReflow();
      currentState = ERROR_STATE;
    }
  } else {
    Input = newTemp;
    thermocoupleErrorCount = 0;  // Reset on success
  }
}

double getTemperatureInput() {
  return Input;
}

int getThermocoupleErrorCount() {
  return thermocoupleErrorCount;
}

void resetThermocoupleErrorCount() {
  thermocoupleErrorCount = 0;
}

Adafruit_MAX31855* getThermocoupleInstance() {
  return &maxthermo;
}

