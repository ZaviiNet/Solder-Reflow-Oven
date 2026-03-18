/*
 * Pin Definitions for Raspberry Pi Pico W
 * Solder Reflow Oven Controller
 */

#ifndef PINS_H
#define PINS_H

// MAX31855 Thermocouple Amplifier Pins
#define THERMO_CLK 2    // GPIO2 - SPI Clock
#define THERMO_CS 3     // GPIO3 - Chip Select
#define THERMO_DO 4     // GPIO4 - MISO (Data Out)

// Solid State Relay Control Pin
#define SSR_PIN 5       // GPIO5 - SSR Control (Heater)

#endif // PINS_H

