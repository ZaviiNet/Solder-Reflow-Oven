/*
 * LED Status Control
 * Manages onboard LED for status indication
 */

#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "config.h"

// Initialize LED
void initLED();

// Update LED based on current state
void updateLED();

// Set LED state directly
void setLED(bool state);

#endif // LED_CONTROL_H

