/*
 * Configuration Storage Implementation
 */

#include "storage.h"
#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// Forward declarations for external variables
extern double preheatTemp, soakTemp, reflowTemp, cooldownTemp;
extern unsigned long preheatTime, soakTime, reflowTime, cooldownTime;

// WiFi credentials
static char storedSSID[32] = "";
static char storedPassword[64] = "";

// Forward declaration for PID functions
extern double getPIDKp();
extern double getPIDKi();
extern double getPIDKd();
extern void setPIDTunings(double kp, double ki, double kd);

void initStorage() {
  Serial.println("Initializing LittleFS...");
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS, formatting...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("ERROR: LittleFS format failed!");
      return;
    }
  }
  Serial.println("LittleFS mounted successfully");
}

void loadConfigFromEEPROM() {
  // Check if config file exists
  if (!LittleFS.exists(CONFIG_FILE)) {
    Serial.println("No configuration file found, using defaults");
    return;
  }

  File configFile = LittleFS.open(CONFIG_FILE, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return;
  }

  Serial.println("Loading configuration from flash...");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();

  if (error) {
    Serial.print("Failed to parse config: ");
    Serial.println(error.c_str());
    return;
  }

  // Load reflow parameters
  preheatTemp = doc["preheatTemp"] | preheatTemp;
  soakTemp = doc["soakTemp"] | soakTemp;
  reflowTemp = doc["reflowTemp"] | reflowTemp;
  cooldownTemp = doc["cooldownTemp"] | cooldownTemp;
  preheatTime = doc["preheatTime"] | preheatTime;
  soakTime = doc["soakTime"] | soakTime;
  reflowTime = doc["reflowTime"] | reflowTime;
  cooldownTime = doc["cooldownTime"] | cooldownTime;

  // Load PID parameters
  double kp = doc["Kp"] | getPIDKp();
  double ki = doc["Ki"] | getPIDKi();
  double kd = doc["Kd"] | getPIDKd();
  setPIDTunings(kp, ki, kd);

  // Load WiFi credentials
  const char* ssid = doc["wifiSSID"];
  const char* password = doc["wifiPassword"];
  if (ssid) {
    strncpy(storedSSID, ssid, sizeof(storedSSID) - 1);
    storedSSID[sizeof(storedSSID) - 1] = '\0';
  }
  if (password) {
    strncpy(storedPassword, password, sizeof(storedPassword) - 1);
    storedPassword[sizeof(storedPassword) - 1] = '\0';
  }

  Serial.println("Configuration loaded successfully from flash!");
  Serial.print("PID Values - Kp: "); Serial.print(kp);
  Serial.print(", Ki: "); Serial.print(ki);
  Serial.print(", Kd: "); Serial.println(kd);
  if (strlen(storedSSID) > 0) {
    Serial.print("Stored WiFi SSID: ");
    Serial.println(storedSSID);
  }
}

void saveConfigToEEPROM() {
  Serial.println("Saving configuration to flash...");

  if (!LittleFS.begin()) {
    Serial.println("ERROR: LittleFS not mounted!");
    return;
  }

  JsonDocument doc;

  // Save reflow parameters
  doc["preheatTemp"] = preheatTemp;
  doc["soakTemp"] = soakTemp;
  doc["reflowTemp"] = reflowTemp;
  doc["cooldownTemp"] = cooldownTemp;
  doc["preheatTime"] = preheatTime;
  doc["soakTime"] = soakTime;
  doc["reflowTime"] = reflowTime;
  doc["cooldownTime"] = cooldownTime;

  // Save PID parameters
  doc["Kp"] = getPIDKp();
  doc["Ki"] = getPIDKi();
  doc["Kd"] = getPIDKd();

  // Save WiFi credentials
  doc["wifiSSID"] = storedSSID;
  doc["wifiPassword"] = storedPassword;

  File configFile = LittleFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  if (serializeJson(doc, configFile) == 0) {
    Serial.println("Failed to write config");
  } else {
    Serial.println("Configuration saved to flash successfully!");
    Serial.print("Saved WiFi SSID: ");
    Serial.println(storedSSID);
  }

  configFile.close();
}

void getStoredWiFiCredentials(char* ssid, char* password, size_t ssidLen, size_t passwordLen) {
  strncpy(ssid, storedSSID, ssidLen - 1);
  ssid[ssidLen - 1] = '\0';
  strncpy(password, storedPassword, passwordLen - 1);
  password[passwordLen - 1] = '\0';
}

void setStoredWiFiCredentials(const char* ssid, const char* password) {
  strncpy(storedSSID, ssid, sizeof(storedSSID) - 1);
  storedSSID[sizeof(storedSSID) - 1] = '\0';
  strncpy(storedPassword, password, sizeof(storedPassword) - 1);
  storedPassword[sizeof(storedPassword) - 1] = '\0';
}

