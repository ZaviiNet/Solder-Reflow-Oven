/*
 * ESP8266 NodeMCU + MAX31855 Thermocouple Example
 * 
 * This is a simple example showing how to wire and read from
 * an Adafruit MAX31855 thermocouple amplifier with a NodeMCU ESP8266.
 * 
 * Wiring:
 * -------
 * MAX31855 -> NodeMCU ESP8266
 * VCC  -> 3.3V
 * GND  -> GND
 * CLK  -> D5 (GPIO14)
 * CS   -> D4 (GPIO2)
 * DO   -> D6 (GPIO12)
 * 
 * Connect K-type thermocouple to MAX31855 T+ and T- terminals
 * 
 * Library Required:
 * - Adafruit MAX31855 library (Install via Library Manager)
 */

#include <Adafruit_MAX31855.h>

// Pin definitions for NodeMCU
#define MAXCLK  D5  // GPIO14
#define MAXCS   D4  // GPIO2  
#define MAXDO   D6  // GPIO12

// Initialize the MAX31855 with software SPI
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("MAX31855 Thermocouple Test - NodeMCU ESP8266");
  
  // Wait for MAX31855 to stabilize
  delay(500);
  
  // Test initial reading
  double c = thermocouple.readCelsius();
  if (isnan(c)) {
    Serial.println("ERROR: Thermocouple fault detected!");
    Serial.println("Check wiring and connections.");
  } else {
    Serial.println("MAX31855 initialized successfully!");
  }
}

void loop() {
  // Read temperature in Celsius
  double celsius = thermocouple.readCelsius();
  
  if (isnan(celsius)) {
    Serial.println("Thermocouple read error!");
  } else {
    // Also read internal temperature (for diagnostics)
    double internal = thermocouple.readInternal();
    
    Serial.print("Thermocouple: ");
    Serial.print(celsius);
    Serial.print(" °C / ");
    Serial.print(celsius * 9.0 / 5.0 + 32.0);
    Serial.print(" °F");
    
    Serial.print("  |  Internal: ");
    Serial.print(internal);
    Serial.println(" °C");
  }
  
  delay(1000);  // Read every second
}
