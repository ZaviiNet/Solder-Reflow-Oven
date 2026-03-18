/*
 * Configuration Storage (LittleFS)
 * Handles persistent storage of settings
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

// Initialize storage system
void initStorage();

// Load configuration from flash
void loadConfigFromEEPROM();

// Save configuration to flash
void saveConfigToEEPROM();

// Get stored Wi-Fi credentials
void getStoredWiFiCredentials(char* ssid, char* password, size_t ssidLen, size_t passwordLen);

// Set stored Wi-Fi credentials
void setStoredWiFiCredentials(const char* ssid, const char* password);

#endif // STORAGE_H

