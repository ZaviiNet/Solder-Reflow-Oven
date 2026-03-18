/*
 * Temperature Sensor Management
 * Handles MAX31855 thermocouple reading and error detection
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <Arduino.h>
#include <Adafruit_MAX31855.h>

// Initialize the temperature sensor
void initTemperatureSensor();

// Read temperature from MAX31855 with error handling
void readTemperature();

// Get the current temperature input
double getTemperatureInput();

// Get the thermocouple error count
int getThermocoupleErrorCount();

// Reset the thermocouple error count
void resetThermocoupleErrorCount();

// Get the MAX31855 instance (for external access if needed)
Adafruit_MAX31855* getThermocoupleInstance();

#endif // TEMPERATURE_H

