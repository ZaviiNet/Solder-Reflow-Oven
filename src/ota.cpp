/*
 * Over-The-Air (OTA) Update Implementation
 * Uses the earlephilhower Arduino-Pico built-in ArduinoOTA library
 */

#include "ota.h"
#include "config.h"
#include <ArduinoOTA.h>

static bool otaInProgress = false;

void initOTA() {
  // Set the hostname used for mDNS discovery (reflow-oven.local)
  ArduinoOTA.setHostname(OTA_HOSTNAME);

  // Uncomment the following line to require a password for OTA uploads:
  // ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "firmware" : "filesystem";
    Serial.println("OTA: Starting update (" + type + ")...");
  });

  ArduinoOTA.onEnd([]() {
    otaInProgress = false;
    Serial.println("\nOTA: Update complete! Rebooting...");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA: %u%%\r", (progress * 100) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    otaInProgress = false;
    Serial.printf("OTA Error [%u]: ", error);
    if      (error == OTA_AUTH_ERROR)    Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)     Serial.println("End Failed");
    else                                 Serial.println("Unknown");
  });

  ArduinoOTA.begin();

  Serial.println("OTA: ArduinoOTA ready");
  Serial.print("OTA: Hostname: ");
  Serial.print(OTA_HOSTNAME);
  Serial.println(".local");
}

void processOTA() {
  ArduinoOTA.handle();
}

bool isOTAInProgress() {
  return otaInProgress;
}
