/*
 * Web Server and HTTP Handlers
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>

// Setup web server and routes
void setupWebServer();

// Process web server client requests (call in loop)
void processWebServerRequests();

// Get web server instance
WebServer* getWebServer();

// Web server handler functions
void handleRoot();
void handleStatus();
void handleGetConfig();
void handleSetConfig();
void handleStart();
void handleStop();
void handleDataLog();
void handleTunePID();
void handleGetConsoleLog();
void handleCaptivePortal();
void handleWiFiSave();
void handleNotFound();
void handleGetPresets();
void handleLoadPreset();
void handleSavePreset();
void handleDeletePreset();

#endif // WEB_SERVER_H

