/*
 * WiFi Setup and Captive Portal
 */

#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>

// Initialize WiFi
void setupWiFi();

// Setup captive portal
void setupCaptivePortal();

// Check if captive portal is active
bool isCaptivePortalActive();

// Process DNS requests (call in loop)
void processDNSRequests();

// Get DNS server instance
DNSServer* getDNSServer();

#endif // WIFI_SETUP_H

