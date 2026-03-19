/*
 * Over-The-Air (OTA) Update Module
 * Supports ArduinoOTA (IDE/PlatformIO network upload)
 */

#ifndef OTA_H
#define OTA_H

#include <Arduino.h>

// Initialize ArduinoOTA (call after WiFi is connected)
void initOTA();

// Process OTA requests (call in loop)
void processOTA();

// Returns true while an OTA update is being received
bool isOTAInProgress();

#endif // OTA_H
